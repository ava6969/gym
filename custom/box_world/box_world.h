//
// Created by Ben Giacalone on 6/30/2021.
//

#ifndef GYMENV_BOX_WORLD_H
#define GYMENV_BOX_WORLD_H

#include "../../env.h"
#include "../../spaces/box.h"
#include "../../spaces/discrete.h"
#include "../../common/rendering.h"
#include <eigen3/unsupported/Eigen/CXX11/Tensor>

namespace gym {

    /// Helper class that stores a color.
    class BoxColor {
    public:
        int r, g, b;

        bool operator==(const BoxColor &other) const {
            return r == other.r && g == other.g && b == other.b;
        }
    };

    /**
     * A CPP implementation of the Box World environment, based on https://github.com/nathangrinsztajn/Box-World.
     *
     * The observation space is a Box space of (board_size + 2) x (board_size + 2) x 3.
     * The action space is a Discrete space of 4:
     * 0:                   move up
     * 1:                   move down
     * 2:                   move left
     * 3:                   move right
     *
     * The following config parameters are available:
     * board_size:          size of the board
     * goal_length:         length of the goal path
     * num_distractor:      number of distractor branches
     * distractor_length:   length of each distractor branch
     * step_cost:           reward per step
     * reward_gem:          reward for getting a gem
     * reward_dead:         reward for dying
     * reward_correct_key   reward for getting a correct key
     * reward_wrong_key     reward for getting a wrong key
     * max_steps            maximum number of steps before terminating
     */
    class BoxWorld : public Env<Eigen::Tensor<float, 3>, int> {
    private:
        // Constants
        static const int SCALE = 10;
        static const int MAX_COLORS = 20;
        static constexpr BoxColor AGENT_COLOR{128, 128, 128};
        static constexpr BoxColor GOAL_COLOR{255, 255, 255};
        static constexpr BoxColor BACKGROUND_COLOR{220, 220, 220};
        static constexpr BoxColor COLORS[MAX_COLORS] = {
                BoxColor{0, 0, 117},
                BoxColor{230, 190, 255},
                BoxColor{170, 255, 195},
                BoxColor{255, 250, 200},
                BoxColor{255, 216, 177},
                BoxColor{250, 190, 190},
                BoxColor{240, 50, 230},
                BoxColor{145, 30, 180},
                BoxColor{67, 99, 216},
                BoxColor{66, 212, 244},
                BoxColor{60, 180, 75},
                BoxColor{191, 239, 69},
                BoxColor{255, 255, 25},
                BoxColor{245, 130, 49},
                BoxColor{230, 25, 75},
                BoxColor{128, 0, 0},
                BoxColor{154, 99, 36},
                BoxColor{128, 128, 0},
                BoxColor{70, 153, 144},
                BoxColor{100, 70, 0}
        };
        static constexpr int CHANGE_COORDINATES[][2] = {
                {-1, 0},
                {1,  0},
                {0,  -1},
                {0,  1}
        };

        // Environment properties
        int m_BoardSize;
        int m_NumPairs;
        int m_NumColors;
        std::vector<BoxColor> m_Colors;
        int m_MaxSteps;
        int m_GoalLength;
        int m_NumDistractor;
        int m_DistractorLength;

        float m_StepCost;
        float m_RewardGem;
        float m_RewardDead;
        float m_RewardCorrectKey;
        float m_RewardWrongKey;

        // Used during environment execution
        int m_NumEnvSteps;
        std::vector<std::vector<BoxColor>> m_World;
        std::vector<int> m_PlayerPosition;
        std::vector<BoxColor> m_DeadEnds;
        std::vector<BoxColor> m_CorrectKeys;
        BoxColor m_OwnedKey;

        // Rendering variables
        std::unique_ptr<class Viewer> m_Viewer;
        std::vector<std::vector<sf::Vector2f>> m_GridVertices;

        // Generates world from a seed.
        std::tuple<
            std::vector<std::vector<BoxColor>>,
            std::vector<int>,
            std::vector<BoxColor>,
            std::vector<BoxColor>
        > gen_world();

        // Checks if a space is empty.
        bool is_empty(BoxColor space);

        // Updates the color of a space after an agent moves.
        void update_color(std::vector<int> previous_pos, std::vector<int> new_pos);

        // Generates random key,lock pairs locations in the environment.
        std::tuple<
            std::vector<std::vector<int>>,
            std::vector<std::vector<int>>,
            std::vector<int>,
            std::vector<int>
        > sample_pair_locations(int num_pair);

    public:
        BoxWorld(
            int board_size=14,
            int goal_length=5,
            int num_distractor=2,
            int distractor_length=2,
            int num_colors=MAX_COLORS,
            float step_cost=0,
            float reward_gem=10,
            float reward_dead=0,
            float reward_correct_key=1,
            float reward_wrong_key=-1
        );

        StepResponse<Eigen::Tensor<float, 3>> step(int const& action) noexcept final;

        Eigen::Tensor<float, 3> reset() noexcept final;

        void render(RenderType type) final;
    };
}

#endif //GYMENV_BOX_WORLD_H