//
// Created by dewe on 5/21/22.
//

//
// Created by Ben Giacalone on 8/28/2021.
//

#include "catch.hpp"
#include "custom/procgen/procgen.h"
#include "opencv2/opencv.hpp"
#include "torch/torch.h"

TEST_CASE("Testing Rendering Procgen") {

    gym::BaseProcgenEnv::Option opt;
    opt.env_name = "coinrun";
    opt.start_level = 0;
    opt.num_levels = 1;
    opt.resource_root = "/home/dewe/sam/gym/custom/procgen/data/assets/";
    opt.distribution_mode = "hard";

    gym::ProcgenEnv env(opt);

    for (int i = 0; i < 2; i++){
        auto s = env.reset();
        float rew;
        while (true) {
            env.render(gym::RenderType::HUMAN);
            auto resp = env.step( torch::randint(15, {1}).item<int>() );
            rew += resp.reward;
            if(resp.done)
                break;

            s = resp.observation;
        }
        printf("returns: %f\n", rew);
    }

}