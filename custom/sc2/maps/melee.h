#pragma once
//
// Created by dewe on 7/14/22.
//


#include "lib.h"


namespace sc2{
    struct Melee : Map{
        Melee(){
            directory = "Melee";
            download = "https://github.com/Blizzard/s2client-proto#map-packs";
            players = 2;
            game_steps_per_episode = 16 * 60 * 30 ; // 30 minute limit
        }
    };

    struct Flat32 : Melee{
        Flat32(){
            filename = "Flat32";
        }
    };
}

