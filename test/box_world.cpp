//
// Created by Ben Giacalone on 8/28/2021.
//

#include "catch.hpp"
#include <custom/box_world/box_world.h>


TEST_CASE("Testing Rendering") {
    gym::BoxWorld env;

    auto s = env.reset();
    auto action_count = env.actionSpace()->size()[0];
    while (true) {
        auto a = rand() % action_count;
        auto resp = env.step(a);

        env.render(gym::RenderType::HUMAN);

        if(resp.done)
            break;
        s = resp.m_NextObservation;
    }
}