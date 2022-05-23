//
// Created by dewe on 5/21/22.
//

#pragma once
#include "box_world.h"

namespace gym {

    class RandomBoxWorld : public BoxWorld{

    public:
        struct Option{
            int board_size=12,
                max_steps=3000,
                num_colors=20;
            std::vector<int > list_goal_lengths={5},
                    list_num_distractors={2},
                    list_distractor_lengths={2};

            float step_cost=0,
            reward_gem=10,
            reward_dead=0,
            reward_correct_key=1,
            reward_wrong_key=-1;
            bool verbose=false;
        };

        explicit RandomBoxWorld(RandomBoxWorld::Option const& );

        std::tuple<torch::Tensor, std::array<int, 2>, std::vector< BColor>, std::vector<BColor>>  world_gen()
        override;

    private:
        Option opt;

        void sample_config();
    };

} // gym