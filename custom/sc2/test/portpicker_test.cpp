//
// Created by dewe on 9/7/22.
//

#include "../lib/util.h"
#include "catch2/catch.hpp"

TEST_CASE("Port Test"){
    SECTION("test NonContiguous Reservation") {
        auto num_port = GENERATE(range(1, 10));
        auto reserved = sc2::pickUnusedPorts(num_port);
        REQUIRE(reserved.size() == num_port);
        std::cout << " reserved " << num_port << " non contiguous ports\n";
        for(auto const& port: reserved)
            std::cout << " reserved port = " << port << "\n";

        REQUIRE(PORTS::RANDOM.size() == num_port);
        sc2::returnPorts(reserved);
        REQUIRE(PORTS::RANDOM.empty());
    }

    SECTION("test Contiguous Reservation") {
        auto num_port = GENERATE(range(2, 5));
        auto reserved = sc2::pickContiguousUnusedPort(num_port);
        REQUIRE(reserved.size() == num_port);

        std::cout << " reserved " << num_port << " contiguous ports\n";
        for(auto const& port: reserved)
            std::cout << " reserved port = " << port << "\n";

        REQUIRE(PORTS::CONTIGUOUS.size() == num_port-1);
        REQUIRE(PORTS::RANDOM.size() == 1);
        sc2::returnPorts(reserved);
        REQUIRE(PORTS::RANDOM.empty());
        REQUIRE(PORTS::CONTIGUOUS.empty());
    }

    SECTION("test Invalid Reservation"){
        REQUIRE_THROWS(sc2::pickUnusedPorts(0));
    }

    SECTION("test Invalid Contiguous Reservation"){
        REQUIRE_THROWS(sc2::pickContiguousUnusedPort(0));
    }
}

