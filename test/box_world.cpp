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

        env.render(gym::RenderType::HUMAN);

        if(resp.done)
            break;
        s = resp.observation;
    }
}

TEST_CASE("Testing Rendering Control") {
    gym::BoxWorld env({});

    auto s = env.reset();
    while (true) {
        env.render(gym::RenderType::HUMAN);

        char key{};
        std::cin >> key;
        int action  = 0;

        switch (key) {
            case 'd': // Left
                action = 2;
                break;
            case 'w': // Up
                action = 0;
                break;
            case 'a': // Right
                action = 3;
                break;
            case 's': // Down
                action = 1;
                break;
        }

        auto resp = env.step( action );

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

        env.render(gym::RenderType::HUMAN);
        cv::waitKey(100);

        auto resp = env.step( torch::randint(4, {1}).item<int>() );
        returns += resp.reward;


        if(resp.done)
            break;
        s = resp.observation;
    }
    std::cout << returns << "\n";
}