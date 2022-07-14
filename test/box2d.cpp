//
// Created by dewe on 7/1/22.
//

#include "box2d/lunarlandar.h"
#include "catch2/catch.hpp"

TEST_CASE("LunarLander System Response") {
    gym::LunarLandarEnv<false> env;
    env.seed(1);

    auto obs = env.reset();
    std::vector<int> actions{
            3, 3, 3, 3, 3, 3, 3, 0, 0, 0
    };

    std::vector<double> rewards{
            -0.27813976299282106, -0.0694419774124799, -0.20630818736307674, 0.2700734135036964, 0.3404690288714687,
            0.3945944044274665, -0.9035419634296591, -1.4619128401170087, -1.4265718086537902, -1.3896685530229433
    };

    std::vector<std::vector<double> > _obs{
            std::vector<double>{-0.0104352 ,  1.386338  , -0.52185816, -0.5590977 ,  0.01007239, 0.07949293,
                                0. ,  0.},
            std::vector<double>{-0.01552239,  1.3731672 , -0.5104059 , -0.58538127,  0.01174381, 0.03343168,
                                0.        ,  0.        },
            std::vector<double>{-0.02054873,  1.3593993 , -0.5027837 , -0.6119044 ,  0.01188598, 0.00284356,
                                0.        ,  0.        },
            std::vector<double>{-0.02548409,  1.3450406 , -0.49135008, -0.6381513 ,  0.00973362,
                                -0.04305113,  0.        ,  0.        },
            std::vector<double>{-0.03033552,  1.3300725 , -0.48084456, -0.6652189 ,  0.00547887,
                                -0.08510326,  0.        ,  0.        },
            std::vector<double>{-3.5098076e-02,  1.3144987e+00, -4.6969527e-01, -6.9216710e-01,
                                -1.0072194e-03, -1.2973361e-01,  0.0000000e+00,  0.0000000e+00},
            std::vector<double>{-0.0397747 ,  1.2983317 , -0.45891786, -0.71856254, -0.00964855,
                                -0.17284243,  0.        ,  0.        },
            std::vector<double>{-0.04445105,  1.2815657 , -0.45889157, -0.7452364 , -0.01828805,
                                -0.17280589,  0.        ,  0.        },
            std::vector<double>{-0.0491271 ,  1.2642007 , -0.45886594, -0.77190983, -0.02692663,
                                -0.17278771,  0.        ,  0.        },
            std::vector<double>{-0.05380287,  1.2462367 , -0.4588408 , -0.79858345, -0.03556389,
                                -0.1727612 ,  0.        ,  0.        },
    };

    REQUIRE_THAT(obs,
                 Catch::Matchers::Approx(
                         std::vector<double>{
                             -0.00525675,1.3989172,-0.5324787,-0.53348106,0.00609814,0.12061419,0.,0.}).margin(0.001));

    for (int i = 0; i < actions.size(); i++) {
        auto response = env.step(actions[i]);
        REQUIRE_THAT(response.observation, Catch::Matchers::Approx(_obs[i]).margin(0.001));
        float r = rewards[i];
        REQUIRE(response.reward == Approx(r).margin(0.01) );
        REQUIRE(response.done == false);
    }

}

TEST_CASE("CarRacing System Response") {
    gym::CarRacing env(true, 0);
    env.seed(1);

    auto obs = env.reset();
    std::vector< std::vector<float>> actions{
            {-0.2262236 ,  0.8304612 ,  0.71231914},
            {0.7524564 , 0.4881306 , 0.36453792},
            {0.57255024, 0.44667456, 0.24010265},
            {0.63420546, 0.45146745, 0.12034852},
            {-0.7817756 ,  0.8656819 ,  0.93485993},
            {-0.48389253,  0.6668109 ,  0.5573828},
            {-0.15000337,  0.70695865,  0.98720753},
            {0.8893631 , 0.5200643 , 0.79352736},
            {-0.7790055 ,  0.99366313,  0.6389408},
            {-0.63174844,  0.53943366,  0.932563}
    };

    std::vector<float> rewards{
            7.1463768115942035, -0.09999999999999964, -0.09999999999999964, -0.09999999999999964, -0.09999999999999964,
            -0.09999999999999964, -0.09999999999999964, -0.09999999999999964, -0.09999999999999964, -0.09999999999999964
    };

    for (int i = 0; i < actions.size(); i++) {
        auto response = env.step(actions[i]);
        float r = rewards[i];
        REQUIRE(response.reward == Approx(r).margin(0.01) );
        REQUIRE(response.done == false);
    }

}