//
// Created by Ben Giacalone on 6/30/2021.
//
#include "common/utils.h"
#include <numeric>
#include "box_world.h"

namespace gym {
    gym::BoxWorld::BoxWorld(Option const &opt) {
        verbose = opt.verbose;
        n = opt.n;
        m_GoalLength = opt.goal_length;
        m_NumDistractor = opt.num_distractor;
        m_DistractorLength = opt.distractor_length;

        m_NumPairs = opt.goal_length - 1 + opt.distractor_length * opt.num_distractor;

        m_StepCost = opt.step_cost;
        m_RewardGem = opt.reward_gem;
        m_RewardDead = opt.reward_dead;
        m_RewardCorrectKey = opt.reward_correct_key;
        m_RewardWrongKey = opt.reward_wrong_key;

        // Generate vertices for rendering
        for (int x = 0; x < n + 2; x++) {
            for (int y = 0; y < n + 2; y++) {
                m_GridVertices.push_back({
                                                 sf::Vector2f{(float) x, (float) y},
                                                 sf::Vector2f{(float) x + 1, (float) y},
                                                 sf::Vector2f{(float) x + 1, (float) y + 1},
                                                 sf::Vector2f{(float) x, (float) y + 1},
                                         });
            }
        }

        // Load colors
        if ( opt.num_colors < m_GoalLength - 1 + m_DistractorLength * m_NumDistractor or  opt.num_colors > 20)
            throw std::runtime_error("Error: Bad number of possible colors.");
        m_NumColors = opt.num_colors;

        m_Colors.resize(m_NumColors);
        for (int i = 0; i < m_NumColors; i++)
            m_Colors[i] = COLORS[i];

        m_MaxSteps = opt.max_steps;
        // Generate observation and action spaces

        m_ObservationSpace = makeBoxSpace<float>(0, 1, {3, n+2, n+2});
        m_ActionSpace = makeDiscreteSpace(4);

        // Generate the world
        m_OwnedKey = {0, 0, 0};
        Env< torch::Tensor, int>::seed(std::nullopt);
        BoxWorld::reset();
        m_NumEnvSteps = 0;

        // Create World State
        m_World = torch::empty({3, n + 2, n + 2}, torch::kUInt8);
    }

    gym::StepResponse<torch::Tensor> gym::BoxWorld::step(const int &action) noexcept {

        auto world = m_World.permute({1, 2, 0});

        int new_action = action;
        auto change = CHANGE_COORDINATES[new_action];
        std::array<int, 2> new_position = {m_PlayerPosition[0] + change[0], m_PlayerPosition[1] + change[1]};
        auto current_position = m_PlayerPosition;

        auto reward = m_StepCost;
        m_NumEnvSteps += 1;
        auto done = m_NumEnvSteps == m_MaxSteps;

        bool possible_move = false;
        // Checks if this location is not on the board
        if (new_position[0] < 1 || new_position[0] >= n + 1 || new_position[1] < 1 ||
            new_position[1] >= n + 1)
            possible_move = false;
            // Checks if nothing is at this location
        else if (is_empty(world[new_position[0]][new_position[1]]))
            possible_move = true;
            // Checks if the space is either a key or a lock
        else if (new_position[1] == 1 || is_empty(world[new_position[0]][new_position[1] - 1])) {
            // If the space is a key
            if (is_empty(world[new_position[0]][new_position[1] + 1])) {
                possible_move = true;
                world[0][0] = world[new_position[0]][new_position[1]];
                m_OwnedKey = toColor(world[0][0]);
                if (torch::equal(toTensor(m_OwnedKey), GOAL_COLOR)) {
                    reward += m_RewardGem;
                    done = true;
                } else if (std::find(m_DeadEnds.begin(), m_DeadEnds.end(), m_OwnedKey) != m_DeadEnds.end()) {
                    reward += m_RewardDead;
                    done = true;
                } else if (std::find(m_CorrectKeys.begin(), m_CorrectKeys.end(), m_OwnedKey) != m_CorrectKeys.end())
                    reward += m_RewardCorrectKey;
                else
                    reward += m_RewardWrongKey;
            }
        }
                // Otherwise, it is a lock
        else {
            // If the space matches an owned key, move into it
            if ( torch::equal( world[new_position[0]][new_position[1]], toTensor(m_OwnedKey) ) )
                possible_move = true;
            else{
                possible_move = false;
                if(verbose){
                    auto c = world[new_position[0]][new_position[1]];
                    printf("lock color is [%i, %i, %i], but owned key is [%i , %i, %i]\n",
                            c[0].item<int>(), c[1].item<int>(), c[2].item<int>(),
                            m_OwnedKey[0], m_OwnedKey[1], m_OwnedKey[2]);
                }
            }

        }


        // If it's possible to move, update the player position
        if (possible_move) {
            m_PlayerPosition = new_position;
            update_color(world, current_position, new_position);
        }

        m_World = world.permute({2, 0, 1});
        return {m_World / 255, reward, done, {}};;
    }

    torch::Tensor gym::BoxWorld::reset() noexcept {
        // Generate a new world
        std::tie(m_World, m_PlayerPosition, m_DeadEnds, m_CorrectKeys) = world_gen();
        return m_World / 255;
    }

    bool gym::BoxWorld::is_empty(torch::Tensor space) {
        return torch::equal(space, BACKGROUND_COLOR) ||
               torch::equal(space, AGENT_COLOR);
    }

    void gym::BoxWorld::update_color(torch::Tensor& world, std::array<int, 2> const& previous_pos, std::array<int, 2>  const& new_pos) {
        world[previous_pos[0]][previous_pos[1]] = BACKGROUND_COLOR;
        world[new_pos[0]][new_pos[1]] = AGENT_COLOR;
    }

    std::array<torch::Tensor, 4> gym::BoxWorld::sample_pair_locations(int num_pair) {

        auto _n = this->n;
        std::vector<int> _possibilities( (_n * (_n - 1)) - 1);
        std::iota(_possibilities.begin(), _possibilities.end(), 1);
        std::set<int> possibilities(_possibilities.begin(), _possibilities.end());

        torch::Tensor keys = torch::empty({num_pair, 2}), locks = torch::empty({num_pair, 2});

        for (int k = 0; k < num_pair; k++) {

            auto key = sample<true>(1, possibilities, m_Device);
            auto key_x = floor_div(key, _n - 1) , key_y = key % (_n - 1);
            auto lock_x = key_x, lock_y = key_y + 1;

            possibilities.erase( key_x * (_n - 1) + key_y );
            for (int i = 1; i < std::min<int>(2, _n - 2 - key_y) + 1; i++) {
                possibilities.erase(key_x * (_n - 1) + i + key_y);
            }

            for (int i = 1; i < std::min<int>(2, key_y) + 1; i++) {
                possibilities.erase( key_x * (_n - 1) - i + key_y);
            }

            keys[k] = torch::tensor({key_x, key_y});
            locks[k] = torch::tensor({lock_x, lock_y});
        }

        auto agent_pos = sample<true>(1, possibilities, m_Device);
        possibilities.erase(agent_pos);
        auto first_key = sample<true>(1, possibilities, m_Device);

        return {keys, locks,
                torch::tensor({ floor_div(first_key, _n - 1), first_key % (_n - 1) }),
                torch::tensor({ floor_div(agent_pos, _n - 1), agent_pos % (_n - 1) })};
    }

    std::tuple<torch::Tensor, std::array<int, 2>, std::vector<BColor>, std::vector<BColor>>
    gym::BoxWorld::world_gen() {
        auto world = torch::ones({n, n, 3}, torch::kUInt8) * BACKGROUND_COLOR;

        // Pick colors for intermediate goals and distractors
        std::vector<int> color_indices(m_NumColors), goal_indices(m_GoalLength - 1);
        std::iota(color_indices.begin(), color_indices.end(), 0);
        std::iota(goal_indices.begin(), goal_indices.end(), 0);

        auto goal_colors = sample<false>(m_GoalLength-1, color_indices, m_Device);

        std::vector<int> distractor_possible_colors;
        for (int color = 0; color < m_NumColors; color++) {
            if ( not contains(goal_colors, color) )
                distractor_possible_colors.push_back(color);
        }

        std::vector<std::vector<int>> distractor_colors;
        for (int i = 0; i < m_NumDistractor; i++) {
            auto branch = sample<false>(m_DistractorLength, distractor_possible_colors, m_Device);
            distractor_colors.push_back(branch);
        }

        // Sample where to branch off distractor branches from goal path
        // This line mainly prevents arbitrary distractor path length
        auto distractor_roots = choice(m_NumDistractor, goal_indices );

        // Find legal positions for all pairs
        auto [keys, locks, first_key, agent_pos] = sample_pair_locations(
                m_GoalLength - 1 + m_DistractorLength * m_NumDistractor);

        std::vector<BColor> dead_ends;
        if (m_GoalLength == 1)
            world[first_key[0]][first_key[1]] = GOAL_COLOR;
        else {
            for (int i = 1; i < m_GoalLength; i++) {
                torch::Tensor color;
                if (i == m_GoalLength - 1)
                    color = GOAL_COLOR;
                else
                    color = m_Colors[goal_colors[i]];
                if(verbose){
                    auto gc = goal_colors[i-1];
                    auto gcc = m_Colors[gc];
                    auto lo = locks[i-1];

                    printf("place a key with color [%i, %i, %i] on position (%i, %i)\n",
                           color[0].item<int>(), color[1].item<int>(), color[2].item<int>(),
                                   keys[i-1][0].item<int>(), keys[i-1][1].item<int>());
                    printf("place a lock with color [%i, %i, %i] on (%i, %i)\n",
                           gcc[0].item<int>(),
                           gcc[1].item<int>(),
                           gcc[2].item<int>(),
                           lo[0].item<int>(), lo[1].item<int>());
                }
                world[keys[i - 1][0].item<int>()][keys[i - 1][1].item<int>()] = color;
                world[locks[i - 1][0].item<int>()][locks[i - 1][1].item<int>()] = m_Colors[goal_colors[i - 1]];
            }

            // keys[0] is orphaned key, so this happens outside the loop
            world[first_key[0]][first_key[1]] = m_Colors[goal_colors[0]];
            if(verbose){
                printf("place the first key with color %i on position (%i, %i)\n",
                       goal_colors[0], first_key[0].item<int>(), first_key[1].item<int>());
            }

            // A dead end is the end of a distractor branch, saved as color so it's consistent with world representation.
            // Iterate over distractor branches to place all distractors.
            for (int64_t i = 0; i < distractor_colors.size(); i++) {
                auto distractor_color = distractor_colors[i];
                auto root = distractor_roots[i];

                // Choose x,y locations for keys and locks from keys and locks (previously determined so nothing collides)
                auto start_index = m_GoalLength - 1 + i * m_DistractorLength;
                auto end_index = m_GoalLength - 1 + (i + 1) * m_DistractorLength;
                auto key_distractor = keys.slice(0, start_index, end_index);
                auto lock_distractor = locks.slice(0, start_index, end_index);

                // Determine colors and place key, lock-pairs
                torch::Tensor color_key;
                for (size_t k = 0; k < key_distractor.size(0); k++) {
                    auto key = key_distractor[i];
                    auto lock = lock_distractor[i];
                    torch::Tensor color_lock;
                    if (k == 0)
                        color_lock = m_Colors[goal_colors[root]];
                    else
                        color_lock = m_Colors[distractor_color[k - 1]];

                    color_key = m_Colors[distractor_color[k]];
                    world[key[0].item<int>()][key[1].item<int>()] = color_key;
                    world[lock[0].item<int>()][lock[1].item<int>()] = color_lock;
                }
                dead_ends.push_back( toColor(color_key) );
            }
        }

        // Place an agent
        world[agent_pos[0].item<int>()][agent_pos[1].item<int>()] = AGENT_COLOR;

        // Convert goal colors to rgb so they have the same format as returned world
        std::vector<BColor> goal_colors_rgb;
        for (int goal_color: goal_colors)
            goal_colors_rgb.push_back( toColor(m_Colors[goal_color]) );

        // Add outline to world by padding
        world = torch::nn::functional::pad(world.permute({2, 0, 1}),
                                           torch::nn::functional::PadFuncOptions({1, 1, 1, 1, 0, 0}));
        agent_pos += torch::tensor({1, 1});

        return {world, {agent_pos[0].item().toInt(), agent_pos[1].item().toInt()}, dead_ends, goal_colors_rgb};
    }

    void gym::BoxWorld::render() {
        // If a viewer does not exist, create it
        if (!m_Viewer) {
            m_Viewer = std::make_unique<Viewer>((n + 2) * SCALE, (n + 2) * SCALE, "BoxWorld");
            m_Viewer->setBounds(0, n + 2, 0, n + 2);
        }

        // Render each pixel as a polygon
        for (int x = 0; x < n + 2; x++) {
            for (int y = 0; y < n + 2; y++) {
                auto vertices = m_GridVertices[y * (n + 2) + x];
                auto c = Color{m_World[0][x][y].item<uint8_t>() / 255.0f,
                               m_World[1][x][y].item<uint8_t>() / 255.0f,
                               m_World[2][x][y].item<uint8_t>() / 255.0f};
                m_Viewer->drawPolygon(vertices, {&c});
            }
        }

        m_Viewer->render();

    }

}