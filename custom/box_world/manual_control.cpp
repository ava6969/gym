//
// Created by dewe on 6/11/22.
//

#include "custom/box_world/random_box_world.h"
#include "opencv2/opencv.hpp"

int main(){
    gym::RandomBoxWorld::Option opt;
    opt.board_size = 12;
    opt.list_goal_lengths = {5};
    opt.list_num_distractors = {3};
    opt.list_distractor_lengths = {4};
    opt.num_colors = 20;
    opt.max_steps = 3000;
    opt.reward_dead = 0;
    opt.reward_correct_key = 1;
    opt.reward_wrong_key = -1;
    opt.reward_gem = 10;
    opt.verbose = true;

    gym::RandomBoxWorld env(opt);

    auto s = env.reset();

    float ret = 0;
    std::cout << "0: UP, 1:DOWN, 2:LEFT, 3: RIGHT\n";
    bool _done = false;
    while (not _done) {

        int action;
        env.render();
        action = cv::waitKey() - 49;
        if(action >= 0 and action < 4){
            auto resp = env.step( action );
            _done = resp.done;
            s = resp.observation;
            ret += resp.reward;
            std::cout << "Action: " << action << "Reward:" << resp.reward  << "\n";
        };

    }
    std::cout << "Returns:" << ret  << "\n";
}