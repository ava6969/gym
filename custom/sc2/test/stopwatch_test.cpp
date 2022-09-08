//
// Created by dewe on 9/5/22.
//

#include "catch2/catch.hpp"

#include "../lib/stopwatch.h"

TEST_CASE("Stat Test"){

    SECTION("test Range"){
        sc2::Stat stat;
        stat.add(1);
        stat.add(5);
        stat.add(3);
        REQUIRE(stat.Num() == 3);
        REQUIRE(stat.Sum() == 9);
        REQUIRE(stat.Min() == 1);
        REQUIRE(stat.Max() == 5);
        REQUIRE(stat.avg() == 3);

    }

    SECTION("test Parse"){
        sc2::Stat stat;
        stat.add(1);
        stat.add(3);
        REQUIRE(LOG_STREAM(stat) == "sum: 4, avg: 2, dev: 1, min: 1, max: 3, num: 2");
    }
}


inline static float retv = 0;

struct Four{
    static void four(){
        retv += 0.004;
    }

    static void six(){
        retv += 0.004;
    }
};

TEST_CASE("Stopwatch Test"){
    SECTION("Decorator disabled"){
        setenv("SC2_NO_STOPWATCH", "1", true);
        sc2::StopWatch l_sw;

        auto round = []() -> int {
            return int(2.5);
        };
        REQUIRE( round() == l_sw.decorate<int>("name")(round)() );
    }

    SECTION("Decorator enabled"){
        setenv("SC2_NO_STOPWATCH", "", true);
        sc2::StopWatch l_sw;

        auto round = []()  {
            return int(2.5);
        };

        REQUIRE( round() == l_sw.decorate<int>("name")(round)() );
    }

    SECTION("Test Divide"){
        sc2::StopWatch _sw;
        {
            sc2::With w{_sw("zero")};
        }
        REQUIRE_THAT( _sw.str(), Catch::Matchers::Contains("zero"));
    }

    SECTION("Test Speed"){
        int count = 100;

        sc2::StopWatch _sw;
        auto run = [count, &_sw](){
            for(int i = 0; i < count; i++){
                sc2::With w{_sw("name")};
            }
        };

        for(int i = 0; i < 10; i++){
            _sw.enable();
            {
                sc2::With w{_sw("enabled")};
                run();
            }

            _sw.trace();
            {
                sc2::With w{_sw("trace")};
                run();
            }

            _sw.enable();
            {
                sc2::With w{_sw("disable")};
                _sw.disable();
                run();
            }
        }

        std::cout << _sw.str() << "\n";
    }

    SECTION("testStopwatch"){
        sc2::StopWatch _sw;
        {
            sc2::With w{_sw("one")};
            retv += 0.002;
        }

        {
            sc2::With w{_sw("one")};
            retv += 0.004;
        }

        {
            sc2::With w{_sw("two")};
            {
                sc2::With w_{_sw("three")};
                retv += 0.006;
            }
        }

        _sw.decorate("four")(Four::four)();

        auto foo = [](){
            retv += 0.005;
        };

        _sw.decorate<void>("five")(foo)();

        auto out = _sw.str();
        std::cout << out << "\n";

    }

}