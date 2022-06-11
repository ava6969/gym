//
// Created by Ben Giacalone on 8/28/2021.
//

#include "catch.hpp"
#include "custom/box_world/random_box_world.h"
#include "env.h"
#include "opencv2/opencv.hpp"

TEST_CASE("Testing Rendering") {
    gym::BoxWorld env({});

    auto s = env.reset();
    while (true) {
        auto resp = env.step( torch::randint(4, {1}).item<int>() );

        env.render();

        if(resp.done)
            break;
        s = resp.observation;
    }
}

TEST_CASE("Testing Rendering Random") {
    gym::RandomBoxWorld env({});

    auto s = env.reset();
    auto returns = 0.f;
    while (true) {
        cv::Mat mat(s.size(1), s.size(2), CV_8UC3, s.flatten().data_ptr());
        cv::resize(mat, mat, {128, 128}, cv::INTER_CUBIC);
        cv::imshow("raw", mat);

        env.render();
        cv::waitKey(100);

        auto resp = env.step( torch::randint(4, {1}).item<int>() );
        returns += resp.reward;


        if(resp.done)
            break;
        s = resp.observation;
    }
    std::cout << returns << "\n";
}