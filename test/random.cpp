//
// Created by dewe on 6/30/22.
//

#include "catch.hpp"
#include "common/utils.h"
#include "random/mtrand/mtrand.h"
#include "random/seeding.h"
#include "vector"


TEST_CASE("Tiny NDArray"){     // 1


    auto result = _int_list_from_bigint(hash_seed(1));
    std::vector<unsigned long> actual_result{2739863373, 598274112};
    REQUIRE_THAT(result, Catch::Matchers::Approx(actual_result) );

    np::RandomState state( std::move(result) );

    float  H = 400.f / 30;
    auto a = state.uniform(0, H/2, 12);

    std::vector _result{5.3826029, 3.43000048, 1.27412101, 1.24583188, 5.15014424, 0.34551928, 0.46350806, 3.43450081, 4.59029097, 3.0641873,  6.01340824, 5.16844953};
    REQUIRE_THAT(a, Catch::Matchers::Approx(_result));
}