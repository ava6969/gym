//
// Created by Ben Giacalone on 6/30/2021.
//

#ifndef GYMENV_BOX_WORLD_H
#define GYMENV_BOX_WORLD_H

#include "env.h"
#include "spaces/box.h"
#include "spaces/discrete.h"
#include "common/rendering.h"
#include "torch/torch.h"


#define BoxColor(...) torch::tensor({ __VA_ARGS__ }, torch::kUInt8)

using BColor = std::array<uint8_t, 3>;

namespace gym {

    /**
     * A CPP implementation of the Box World environment, based on https://github.com/mavischer/Box-World/tree/master/gym-boxworld
     *
     * The observation space is a Box space of (n + 2) x (n + 2) x 3.
     * The action space is a Discrete space of 4:
     * 0:                   move up
     * 1:                   move down
     * 2:                   move left
     * 3:                   move right
     *
     * The following config parameters are available:
     * n:          size of the board
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
    class BoxWorld : public Env< torch::Tensor, int> {
    private:

        // Constants
        torch::Tensor m_state;
        static constexpr int SCALE = 10;
        static constexpr int MAX_COLORS = 20;
        inline static const torch::Tensor AGENT_COLOR = BoxColor(128, 128, 128);
        inline static const torch::Tensor GOAL_COLOR = BoxColor(255, 255, 255);
        inline static const torch::Tensor BACKGROUND_COLOR = BoxColor(220, 220, 220);
        inline static const torch::Tensor COLORS[MAX_COLORS] = {
                BoxColor(0, 0, 117),
                BoxColor(230, 190, 255),
                BoxColor(170, 255, 195),
                BoxColor(255, 250, 200),
                BoxColor(255, 216, 177),
                BoxColor(250, 190, 190),
                BoxColor(240, 50, 230),
                BoxColor(145, 30, 180),
                BoxColor(67, 99, 216),
                BoxColor(66, 212, 244),
                BoxColor(60, 180, 75),
                BoxColor(191, 239, 69),
                BoxColor(255, 255, 25),
                BoxColor(245, 130, 49),
                BoxColor(230, 25, 75),
                BoxColor(128, 0, 0),
                BoxColor(154, 99, 36),
                BoxColor(128, 128, 0),
                BoxColor(70, 153, 144),
                BoxColor(100, 70, 0)
        };
        static constexpr int CHANGE_COORDINATES[][2] = {
                {-1, 0},
                {1,  0},
                {0,  -1},
                {0,  1}
        };

        // Environment properties
        int n;
        int m_NumPairs;
        int m_NumColors;
        std::vector<torch::Tensor> m_Colors;
        int m_MaxSteps;

        float m_StepCost;
        float m_RewardGem;
        float m_RewardDead;
        float m_RewardCorrectKey;
        float m_RewardWrongKey;

        // Used during environment execution
        int m_NumEnvSteps;
        torch::Tensor m_World;
        std::array<int, 2> m_PlayerPosition;
        std::vector<BColor> m_DeadEnds;
        std::vector<BColor> m_CorrectKeys;
        BColor m_OwnedKey;

        // Rendering variables
        std::unique_ptr<class Viewer> m_Viewer;
        std::vector<std::vector<sf::Vector2f>> m_GridVertices;

        // Checks if a space is empty.
        static bool is_empty(torch::Tensor space);

        // Updates the color of a space after an agent moves.
        void update_color(torch::Tensor& world, std::array<int, 2> const&, std::array<int, 2>  const& new_pos);

        // Generates random key,lock pairs locations in the environment.
        std::array<torch::Tensor, 4> sample_pair_locations(int num_pair);

        static inline BColor toColor(torch::Tensor const& x){
            return {x[0].item<uint8_t>(),
                    x[1].item<uint8_t>(),
                    x[2].item<uint8_t>()};
        }

        static inline torch::Tensor toTensor(BColor const& x){
            return torch::tensor({x[0], x[1], x[2]});
        }

    protected:
        int m_GoalLength;
        int m_NumDistractor;
        int m_DistractorLength;
        bool verbose;

        // Generates world from a seed.
        virtual std::tuple<torch::Tensor, std::array<int, 2>, std::vector<BColor>, std::vector<BColor>>  world_gen();

    public:

        struct Option{
            int n=12,
             goal_length=5,
             num_distractor=2,
             distractor_length=2,
             max_steps=3000,
             num_colors=MAX_COLORS;
            float step_cost=0,
             reward_gem=10,
             reward_dead=0,
             reward_correct_key=1,
             reward_wrong_key=-1;
            bool verbose= false;
        };

        explicit BoxWorld(Option const&);

        StepResponse<torch::Tensor> step(int const& action) noexcept final;

        torch::Tensor reset() noexcept final;

        void render() final;
    };
}

#endif //GYMENV_BOX_WORLD_H