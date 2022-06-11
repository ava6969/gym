//
// Created by dewe on 5/21/22.
//

#include "common/utils.h"
#include "random_box_world.h"


namespace gym {

    RandomBoxWorld::RandomBoxWorld(RandomBoxWorld::Option const& opt):
            BoxWorld({
                opt.board_size,
                uniformRandom<1>(opt.list_goal_lengths, std::mt19937{std::random_device{}()}),
                uniformRandom<1>(opt.list_num_distractors, std::mt19937{std::random_device{}()}),
                uniformRandom<1>(opt.list_distractor_lengths, std::mt19937{std::random_device{}()}),
                        opt.max_steps,
                        opt.num_colors,
                        opt.step_cost,
                        opt.reward_gem,
                        opt.reward_dead,
                        opt.reward_correct_key,
                        opt.reward_wrong_key,
                        opt.verbose
            }){}

    std::tuple<cv::Mat, mg::Point, std::vector<BColor>, std::vector<BColor>>
    RandomBoxWorld::world_gen(){
        sample_config();
        return BoxWorld::world_gen();
    }

    void RandomBoxWorld::sample_config(){
        m_GoalLength = uniformRandom<1>(opt.list_goal_lengths, m_Device);
        m_NumDistractor = uniformRandom<1>(opt.list_num_distractors, m_Device);
        m_DistractorLength = uniformRandom<1>(opt.list_distractor_lengths, m_Device);
    }

} // gym