#pragma once
//
// Created by dewe on 7/14/22.
//

#include "lib.h"


namespace sc2{
    struct Ladder : Map{
        Ladder(){
            players = 2;
            game_steps_per_episode = 16 * 60 * 30;  // 30 minute limit.
            download = "https://github.com/Blizzard/s2client-proto#map-packs";
        }
    };

    struct Ladder2017Season1 : Map{
        Ladder2017Season1(){
            directory = "Ladder2017Season1";
        }
    };
}
