//
// Created by Ben Giacalone on 6/30/2021.
//

#include <numeric>
#include "box_world.h"

gym::BoxWorld::BoxWorld(
    int board_size,
    int goal_length,
    int num_distractor,
    int distractor_length,
    int num_colors,
    float step_cost,
    float reward_gem,
    float reward_dead,
    float reward_correct_key,
    float reward_wrong_key
) {
    m_BoardSize = board_size;
    m_GoalLength = goal_length;
    m_NumDistractor = num_distractor;
    m_DistractorLength = distractor_length;
    m_NumColors = num_colors;
    m_GoalLength = goal_length - 1 + distractor_length * num_distractor;

    m_StepCost = step_cost;
    m_RewardGem = reward_gem;
    m_RewardDead = reward_dead;
    m_RewardCorrectKey = reward_correct_key;
    m_RewardWrongKey = reward_wrong_key;

    m_MaxSteps = 3000;

    // Generate vertices for rendering
    for (int x = 0; x < m_BoardSize + 2; x++) {
        for (int y = 0; y < m_BoardSize + 2; y++) {
            m_GridVertices.push_back({
                sf::Vector2f {(float)x, (float)y},
                sf::Vector2f {(float)x + 1, (float)y},
                sf::Vector2f {(float)x + 1, (float)y + 1},
                sf::Vector2f {(float)x, (float)y + 1},
            });
        }
    }

    // Load colors
    if (m_NumColors < m_GoalLength - 1 + m_DistractorLength * m_NumDistractor || m_NumColors > 20)
        std::cout << "Error: Bad number of possible colors." << std::endl;
    for (int i = 0; i < m_NumColors; i++)
          m_Colors.push_back(COLORS[i]);

    // Generate observation and action spaces
    Shape obs_shape{ {3, m_BoardSize + 2, m_BoardSize + 2}, torch::kFloat64 };
    m_ObservationSpace = makeBoxSpace<float>(obs_shape);
    m_ActionSpace = makeDiscreteSpace(4);

    // Generate the world
    std::tie(m_World, m_PlayerPosition, m_CorrectKeys, m_DeadEnds) = gen_world();
    m_NumEnvSteps = 0;
}

gym::StepResponse<Eigen::Tensor<float, 3>> gym::BoxWorld::step(const int & action) noexcept {
    int new_action = action;
    auto change = CHANGE_COORDINATES[new_action];
    std::vector<int> new_position = { m_PlayerPosition[0] + change[0], m_PlayerPosition[1] + change[1] };
    auto current_position = m_PlayerPosition;

    auto reward = m_StepCost;
    m_NumEnvSteps += 1;
    auto done = m_NumEnvSteps == m_MaxSteps;

    bool possible_move = false;
    // Checks if this location is not on the board
    if (new_position[0] < 1 || new_position[0] >= m_BoardSize + 1 || new_position[1] < 1 || new_position[1] >= m_BoardSize + 1)
        possible_move = false;
        // Checks if nothing is at this location
    else if (is_empty(m_World[new_position[0]][new_position[1]]))
        possible_move = true;
        // Checks if the space is either a key or a lock
    else if (new_position[1] == 1 || is_empty(m_World[new_position[0]][new_position[1] - 1])) {
        // If the space is a key
        if (is_empty(m_World[new_position[0]][new_position[1] + 1])) {
            possible_move = true;
            m_OwnedKey = m_World[new_position[0]][new_position[1]];
            m_World[0][0] = m_OwnedKey;
            if (m_OwnedKey == GOAL_COLOR) {
                reward += m_RewardGem;
                done = true;
            }
            else if (std::find(m_DeadEnds.begin(), m_DeadEnds.end(), m_OwnedKey) != m_DeadEnds.end()) {
                reward += m_RewardDead;
                done = true;
            }
            else if (std::find(m_CorrectKeys.begin(), m_CorrectKeys.end(), m_OwnedKey) != m_CorrectKeys.end())
                reward += m_RewardCorrectKey;
            else
                reward += m_RewardWrongKey;
        }
            // Otherwise, it is a lock
        else {
            // If the space matches an owned key, move into it
            if (m_World[new_position[0]][new_position[1]] == m_OwnedKey)
                possible_move = true;
        }
    }

    // If it's possible to move, update the player position
    if (possible_move) {
        m_PlayerPosition = new_position;
        update_color(current_position, new_position);
    }

    // Convert the world into a tensor
    Eigen::Tensor<float, 3> obs(3, m_BoardSize + 2, m_BoardSize + 2);
    for (int i = 0; i < m_BoardSize + 2; i++) {
        for (int j = 0; j < m_BoardSize + 2; j++) {
            obs(0, i, j) = m_World[i][j].r;
            obs(1, i, j) = m_World[i][j].g;
            obs(2, i, j) = m_World[i][j].b;
        }
    }

    return {obs, reward, done, {}};;
}

Eigen::Tensor<float, 3> gym::BoxWorld::reset() noexcept {
    // Generate a new world
    std::tie(m_World, m_PlayerPosition, m_CorrectKeys, m_DeadEnds) = gen_world();

    // Convert the world into a matrix
    Eigen::Tensor<float, 3> obs(3, m_BoardSize + 2, m_BoardSize + 2);
    for (int i = 0; i < m_BoardSize + 2; i++) {
        for (int j = 0; j < m_BoardSize + 2; j++) {
            obs(0, i, j) = m_World[i][j].r;
            obs(1, i, j) = m_World[i][j].g;
            obs(2, i, j) = m_World[i][j].b;
        }
    }

    return obs;
}

bool gym::BoxWorld::is_empty(gym::BoxColor space) {
    return space == BACKGROUND_COLOR || space == AGENT_COLOR;
}

void gym::BoxWorld::update_color(std::vector<int> previous_pos, std::vector<int> new_pos) {
    m_World[previous_pos[0]][previous_pos[1]] = BACKGROUND_COLOR;
    m_World[new_pos[0]][new_pos[1]] = AGENT_COLOR;
}

std::tuple<
        std::vector<std::vector<gym::BoxColor>>,
        std::vector<int>,
        std::vector<gym::BoxColor>,
        std::vector<gym::BoxColor>
> gym::BoxWorld::gen_world() {
    // Fill the world with the background color
    std::vector<std::vector<BoxColor>> world(m_BoardSize, std::vector<BoxColor>(m_BoardSize, BACKGROUND_COLOR));

    // A lot of the original Python code relied on functional programming/generators, so
    // the C++ equivalent looks a bit messy.

    // Pick colors for intermediate goals and distractors
    std::vector<int> color_indices(m_NumColors);
    std::iota(color_indices.begin(), color_indices.end(), 0);
    std::vector<int> goal_colors;
    for (int i = 0; i < m_GoalLength - 1; i++) {
        auto c_ind = (int)((std::rand() / (float)RAND_MAX) * color_indices.size());
        goal_colors.push_back(color_indices[c_ind]);
        color_indices.erase(color_indices.begin() + c_ind);
    }

    std::vector<int> distractor_possible_colors;
    for (int i = 0; i < m_NumColors; i++) {
        if (std::find(goal_colors.begin(), goal_colors.end(), i) != goal_colors.end())
            distractor_possible_colors.push_back(i);
    }
    std::vector<std::vector<int>> distractor_colors;
    for (int i = 0; i < m_NumDistractor; i++) {
        std::vector<int> branch;
        for (int j = 0; j < m_DistractorLength; j++) {
            auto d_ind = (int)((std::rand() / (float)RAND_MAX) * distractor_possible_colors.size());
            branch.push_back(distractor_possible_colors[d_ind]);
            distractor_possible_colors.erase(distractor_possible_colors.begin() + d_ind);
        }
        distractor_colors.push_back(branch);
    }

    // Sample where to branch off distractor branches from goal path
    // This line mainly prevents arbitrary distractor path length
    std::vector<int> distractor_roots;
    for (int i = 0; i < m_NumDistractor; i++) {
        int rand_int = (std::rand() / (float)RAND_MAX) * (m_GoalLength - 1);
        distractor_roots.push_back(rand_int);
    }

    // Find legal positions for all pairs
    std::vector<std::vector<int>> keys;
    std::vector<std::vector<int>> locks;
    std::vector<int> first_key;
    std::vector<int> agent_pos;
    std::tie(keys, locks, first_key, agent_pos) = sample_pair_locations(m_GoalLength - 1 + m_DistractorLength * m_NumDistractor);

    std::vector<BoxColor> m_DeadEnds;
    if (m_GoalLength == 1)
        world[first_key[0]][first_key[1]] = GOAL_COLOR;
    else {
        for (int i = 1; i < m_GoalLength; i++) {
            BoxColor color;
            if (i == m_GoalLength - 1)
                color = GOAL_COLOR;
            else
                color = m_Colors[goal_colors[i]];
            world[keys[i - 1][0]][keys[i - 1][1]] = color;
            world[locks[i - 1][0]][locks[i - 1][1]] = m_Colors[goal_colors[i - 1]];
        }

        // keys[0] is orphaned key, so this happens outside the loop
        world[first_key[0]][first_key[1]] = m_Colors[goal_colors[0]];

        // A dead end is the end of a distractor branch, saved as color so it's consistent with world representation.
        // Iterate over distractor branches to place all distractors.
        for (size_t i = 0; i < distractor_colors.size(); i++) {
            auto distractor_color = distractor_colors[i];
            auto root = distractor_roots[i];

            // Choose x,y locations for keys and locks from keys and locks (previously determined so nothing collides)
            auto start_index = m_GoalLength - 1 + i * m_DistractorLength;
            auto end_index = m_GoalLength - 1 + (i + 1) * m_DistractorLength;
            auto key_distractor = std::vector<std::vector<int>>(keys.begin() + start_index, keys.begin() + end_index);
            auto lock_distractor = std::vector<std::vector<int>>(locks.begin() + start_index, locks.begin() + end_index);

            // Determine colors and place key, lock-pairs
            BoxColor color_key;
            for (size_t k = 0; k < key_distractor.size(); k++) {
                auto key = key_distractor[i];
                auto lock = lock_distractor[i];
                BoxColor color_lock;
                if (k == 0)
                    color_lock = m_Colors[goal_colors[root]];
                else
                    color_lock = m_Colors[distractor_color[k - 1]];

                color_key = m_Colors[distractor_color[k]];
                world[key[0]][key[1]] = color_key;
                world[lock[0]][lock[1]] = color_lock;
            }
            m_DeadEnds.push_back(color_key);
        }
    }

    // Place an agent
    world[agent_pos[0]][agent_pos[1]] = AGENT_COLOR;

    // Convert goal colors to rgb so they have the same format as returned world
    std::vector<BoxColor> goal_colors_rgb;
    for (int goal_color : goal_colors)
        goal_colors_rgb.push_back(m_Colors[goal_color]);

    // Add outline to world by padding
    for (int l = 0; l < m_BoardSize; l++) {
        world[l].insert(world[l].begin(), BoxColor {0, 0, 0});
        world[l].push_back(BoxColor {0, 0, 0});
    }
    world.insert(world.begin(), std::vector<BoxColor>(m_BoardSize + 2, BoxColor {0, 0, 0}));
    world.push_back(std::vector<BoxColor>(m_BoardSize + 2, BoxColor {0, 0, 0}));
    agent_pos[0] += 1;
    agent_pos[1] += 1;

    return std::tie(world, agent_pos, m_DeadEnds, goal_colors_rgb);
}
std::tuple<
        std::vector<std::vector<int>>,
        std::vector<std::vector<int>>,
        std::vector<int>,
        std::vector<int>
> gym::BoxWorld::sample_pair_locations(int num_pair) {

    auto n = m_BoardSize;
    std::vector<int> possibilities(n * (n - 1) - 1);
    std::iota(possibilities.begin(), possibilities.end(), 1);
    std::vector<std::vector<int>> keys;
    std::vector<std::vector<int>> locks;

    for (int k = 0; k < num_pair; k++) {
        auto key_index = (int)((std::rand() / (float)RAND_MAX) * possibilities.size());
        auto key = possibilities[key_index];
        auto key_x = key / (n - 1), key_y = key % (n - 1);
        auto lock_x = key_x, lock_y = key_y + 1;
        auto position = std::find(possibilities.begin(), possibilities.end(), key_x * (n - 1) + key_y);
        if (position != possibilities.end())
            possibilities.erase(position);
        for (int i = 1; i < std::min<int>(2, n - 2 - key_y) + 1; i++) {
            position = std::find(possibilities.begin(), possibilities.end(), key_x * (n - 1) + i + key_y);
            if (position != possibilities.end())
                possibilities.erase(position);
        }
        for (int i = 1; i < std::min<int>(2, key_y) + 1; i++) {
            position = std::find(possibilities.begin(), possibilities.end(), key_x * (n - 1) - i + key_y);
            if (position != possibilities.end())
                possibilities.erase(position);
        }
        keys.push_back(std::vector<int> {key_x, key_y});
        locks.push_back(std::vector<int> {lock_x, lock_y});
    }
    auto agent_pos_index = (int)((std::rand() / (float)RAND_MAX) * possibilities.size());
    auto agent_pos_ = possibilities[agent_pos_index];
    possibilities.erase(possibilities.begin() + agent_pos_index);
    auto first_key_index = (int)((std::rand() / (float)RAND_MAX) * possibilities.size());
    auto first_key_ = possibilities[first_key_index];

    auto agent_pos = std::vector<int> { agent_pos_ / (n - 1), agent_pos_ % (n - 1) };
    auto first_key = std::vector<int> { first_key_ / (n - 1), first_key_ % (n - 1) };
    return std::tie(keys, locks, first_key, agent_pos);
}

void gym::BoxWorld::render(RenderType ) {
    // If a viewer does not exist, create it
    if (!m_Viewer) {
        m_Viewer = std::make_unique<Viewer>((m_BoardSize + 2) * SCALE, (m_BoardSize + 2) * SCALE, "BoxWorld");
        m_Viewer->setBounds(0, m_BoardSize + 2, 0, m_BoardSize + 2);
    }

    // Render each pixel as a polygon
    for (int x = 0; x < m_BoardSize + 2; x++) {
        for (int y = 0; y < m_BoardSize + 2; y++) {
            auto vertices = m_GridVertices[y * (m_BoardSize + 2) + x];
            auto c = Color {m_World[x][y].r / 255.0f, m_World[x][y].g / 255.0f, m_World[x][y].b / 255.0f };
            m_Viewer->drawPolygon(vertices, {&c});
        }
    }

    m_Viewer->render();

}