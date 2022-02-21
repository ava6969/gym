//
// Created by dewe on 12/29/21.
//

#include "wrappers/vec_env/vec_frame_stack.h"
#include "wrappers/vec_env/vec_atari.h"
#include "catch.hpp"

#define private public

TEST_CASE("FrameStack"){

    gym::StackedObservation<false> stacker(2, 4, gym::makeBoxSpace<float>(0, 255, {2, 2, 1}));
    c10::IntArrayRef shape = {2, 1, 2, 2};
    auto x = torch::ones(shape);
    auto result = torch::stack({torch::zeros(shape), torch::zeros(shape), torch::zeros(shape), x});
    auto obs = stacker.reset(x);

    REQUIRE(x.dim() == result.dim());

}
