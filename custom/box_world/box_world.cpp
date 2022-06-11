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

        m_ObservationSpace = makeBoxSpace<uint8_t>(0, 255, {n+2, n+2, 3});
        m_ActionSpace = makeDiscreteSpace(4);

        // Generate the world
        m_OwnedKey = {0, 0, 0};
        m_NumEnvSteps = 0;
        Env< cv::Mat, int>::seed(std::nullopt);
        BoxWorld::reset();

    }

    gym::StepResponse<cv::Mat> gym::BoxWorld::step(const int &action) noexcept {

        int new_action = action;
        auto change = CHANGE_COORDINATES[new_action];
        mg::Point newPosition = m_PlayerPosition + change;
        auto current_position = m_PlayerPosition;

        auto reward = m_StepCost;
        m_NumEnvSteps += 1;
        auto done = m_NumEnvSteps == m_MaxSteps;

        bool possible_move = false;
        // Checks if this location is not on the board
        if ( std::ranges::any_of( newPosition < 1 , [](bool x){ return x; }) || std::ranges::any_of(newPosition >= n + 1, [](bool x){ return x; }) )
            possible_move = false;
            // Checks if nothing is at this location
        else if (is_empty( worldAt(newPosition) ) )
            possible_move = true;
            // Checks if the space is either a key or a lock
        else if (newPosition.y == 1 || is_empty( worldAt( { newPosition.x, newPosition.y - 1} ) ) ) {
            // If the space is a key
            if (is_empty( worldAt( { newPosition.x, newPosition.y + 1} ) ) ) {
                possible_move = true;
                m_OwnedKey = worldAtC(newPosition);
                worldAt({0, 0}) = m_OwnedKey;
                if ( m_OwnedKey == GOAL_COLOR ) {
                    reward += m_RewardGem;
                    done = true;
                } else if (std::find(m_DeadEnds.begin(), m_DeadEnds.end(), m_OwnedKey) != m_DeadEnds.end()) {
                    reward += m_RewardDead;
                    done = true;
                } else if (std::find(m_CorrectKeys.begin(), m_CorrectKeys.end(), m_OwnedKey) != m_CorrectKeys.end())
                    reward += m_RewardCorrectKey;
                else{
                    reward += m_RewardWrongKey;
                    if(m_RewardWrongKey != 0)
                        done = true;
                }

            }
        }
                // Otherwise, it is a lock
        else {
            // If the space matches an owned key, move into it
            if ( worldAtC(newPosition) == m_OwnedKey )
                possible_move = true;
            else{
                possible_move = false;
                if(verbose){
                    auto c = worldAtC(newPosition);
                    printf("lock color is [%i, %i, %i], but owned key is [%i , %i, %i]\n",
                            c[0], c[1], c[2], m_OwnedKey[0], m_OwnedKey[1], m_OwnedKey[2]);
                }
            }

        }

        // If it's possible to move, update the player position
        if (possible_move) {
            m_PlayerPosition = newPosition;
            update_color(m_World, current_position, newPosition);
        }
        state = m_World.clone();
        return {state, reward, done, {}};;
    }

    cv::Mat gym::BoxWorld::reset() noexcept {
        // Generate a new world
        std::tie(m_World, m_PlayerPosition, m_DeadEnds, m_CorrectKeys) = world_gen();
        state = m_World.clone();
        return state;
    }

    bool gym::BoxWorld::is_empty(cv::Vec3b const& space) {
        return space == BACKGROUND_COLOR || space == AGENT_COLOR;
    }

    void gym::BoxWorld::update_color(cv::Mat& world, mg::Point const& previous_pos, mg::Point const& new_pos) {
        world.at<cv::Vec3b>(previous_pos.x, previous_pos.y) = BACKGROUND_COLOR;
        world.at<cv::Vec3b>(new_pos.x, new_pos.y) = AGENT_COLOR;
    }

    std::tuple< std::vector<mg::Point>, std::vector<mg::Point>, mg::Point, mg::Point>
            gym::BoxWorld::sample_pair_locations(int num_pair) {

        auto _n = this->n;
        std::vector<int> _possibilities( (_n * (_n - 1)) - 1);
        std::iota(_possibilities.begin(), _possibilities.end(), 1);
        std::set<int> possibilities(_possibilities.begin(), _possibilities.end());

        std::vector<mg::Point> keys(num_pair), locks(num_pair);

        for (int k = 0; k < num_pair; k++) {

            auto key = sample<true>(1, possibilities, m_Device);
            int key_x = floor_div(key, _n - 1) , key_y = key % (_n - 1);
            int lock_x = key_x, lock_y = key_y + 1;

            possibilities.erase( key_x * (_n - 1) + key_y );
            for (int i = 1; i < std::min<int>(2, _n - 2 - key_y) + 1; i++) {
                possibilities.erase(key_x * (_n - 1) + i + key_y);
            }

            for (int i = 1; i < std::min<int>(2, key_y) + 1; i++) {
                possibilities.erase( key_x * (_n - 1) - i + key_y);
            }

            keys[k] = {key_x, key_y};
            locks[k] = {lock_x, lock_y};
        }

        auto agent_pos = sample<true>(1, possibilities, m_Device);
        possibilities.erase(agent_pos);
        auto first_key = sample<true>(1, possibilities, m_Device);

        return {keys, locks,
                { floor_div(first_key, _n - 1), first_key % (_n - 1) },
                { floor_div(agent_pos, _n - 1), agent_pos % (_n - 1) }};
    }

    std::tuple<cv::Mat, mg::Point, std::vector<BColor>, std::vector<BColor>>
    gym::BoxWorld::world_gen() {
        auto world = cv::Mat(n, n, CV_8UC3, BACKGROUND_COLOR);

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
            world.at<BColor>(first_key.x, first_key.y) = GOAL_COLOR;
        else {
            for (int i = 1; i < m_GoalLength; i++) {
                BColor color;
                if (i == m_GoalLength - 1)
                    color = GOAL_COLOR;
                else
                    color = m_Colors[goal_colors[i]];
                if(verbose){
                    auto gc = goal_colors[i-1];
                    auto gcc = m_Colors[gc];
                    auto lo = locks[i-1];

                    printf("place a key with color [%i, %i, %i] on position (%i, %i)\n",
                           color[0], color[1], color[2], keys[i-1].x, keys[i-1].y);
                    printf("place a lock with color [%i, %i, %i] on (%i, %i)\n",
                           gcc[0], gcc[1], gcc[2], lo.x, lo.y);
                }
                world.at<BColor>(keys[i - 1].x, keys[i - 1].y)   = color;
                world.at<BColor>(locks[i - 1].x, locks[i - 1].y) = m_Colors[goal_colors[i - 1]];
            }

            // keys[0] is orphaned key, so this happens outside the loop
            world.at<BColor>(first_key.x, first_key.y)   = m_Colors[goal_colors[0]];
            if(verbose){
                printf("place the first key with color %i on position (%i, %i)\n",
                       goal_colors[0], first_key.x, first_key.y );
            }

            // A dead end is the end of a distractor branch, saved as color so it's consistent with world representation.
            // Iterate over distractor branches to place all distractors.
            for (int64_t i = 0; i < distractor_colors.size(); i++) {
                auto distractor_color = distractor_colors[i];
                auto root = distractor_roots[i];

                // Choose x,y locations for keys and locks from keys and locks (previously determined so nothing collides)
                auto start_index = m_GoalLength - 1 + i * m_DistractorLength;
                auto end_index = m_GoalLength - 1 + (i + 1) * m_DistractorLength;
                auto key_distractor = slice(keys, start_index, end_index);
                auto lock_distractor = slice(locks, start_index, end_index);

                // Determine colors and place key, lock-pairs
                BColor color_key;
                for (size_t k = 0; k < key_distractor.size(); k++) {
                    auto key = key_distractor[k];
                    auto lock = lock_distractor[k];
                    BColor color_lock;
                    if (k == 0)
                        color_lock = m_Colors[goal_colors[root]];
                    else
                        color_lock = m_Colors[distractor_color[k - 1]];

                    color_key = m_Colors[distractor_color[k]];
                    world.at<BColor>( key.x, key.y) = color_key;
                    world.at<BColor>( lock.x, lock.y)  = color_lock;
                }
                dead_ends.push_back( color_key );
            }
        }

        // Place an agent
        world.at<BColor>(agent_pos.x, agent_pos.y) = AGENT_COLOR;

        // Convert goal colors to rgb so they have the same format as returned world
        std::vector<BColor> goal_colors_rgb;
        for (int goal_color: goal_colors)
            goal_colors_rgb.push_back( m_Colors[goal_color] );

        // Add outline to world by padding
        cv::copyMakeBorder( world, world, 1, 1, 1, 1, cv::BORDER_CONSTANT, 0 );
        agent_pos = agent_pos + mg::Point{1, 1};

        return {world, agent_pos, dead_ends, goal_colors_rgb};
    }

    void gym::BoxWorld::render() {
        auto s = m_World.clone();
        cv::resize(s, s, {s.size[0]*SCALE, s.size[0]*SCALE}, 0, 0, cv::INTER_AREA);
        cv::cvtColor(s, s, cv::COLOR_RGB2BGR);
        cv::imshow("world", s);
        cv::waitKey(100);

    }

}