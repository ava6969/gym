//
// Created by dewe on 8/22/21.
//

#include "catch.hpp"
#include "common/misc.h"

using namespace gym;
using namespace gym::space;

TEST_CASE("Discrete Space Init"){

    auto discrete = gym::makeDiscreteSpace(4);

    REQUIRE(discrete->size()[0] == 4);
    REQUIRE(discrete->size().size() == 1);
}

TEST_CASE("Discrete Space Sample"){

    auto discrete = gym::makeDiscreteSpace(2);
    gym::space::Discrete ds(4);

    std::cout << ds << "\n";

    auto matrix = discrete->sample<int>();

//    std::cout << "generated " << matrix << "\n";

    REQUIRE(matrix < 2);
    REQUIRE(ds.n == 4);
}

TEST_CASE("Box Space Init"){

    auto _1dBox = makeBoxSpace<float>(
            make_shape<float>(4));

    auto _2dBox = makeBoxSpace<float>(
            make_shape<float>({4, 4}));

    auto boundedBox = makeBoxSpace<float>({-10, -5, -3},
                                          {10, 5, 3}, 4);


}
