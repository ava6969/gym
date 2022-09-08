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
            m_players = 2;
            game_steps_per_episode = 16 * 60 * 30 ; // 30 minute limit
        }
    };

#define MELEE_SUB_CLASSES(name) \
    struct name : Melee{ \
    name (){ \
            filename = #name; \
        } \
    };

    MELEE_SUB_CLASSES(Flat32)
    MELEE_SUB_CLASSES(Flat48)
    MELEE_SUB_CLASSES(Flat64)
    MELEE_SUB_CLASSES(Flat96)
    MELEE_SUB_CLASSES(Flat128)
    MELEE_SUB_CLASSES(Simple64)
    MELEE_SUB_CLASSES(Simple96)
    MELEE_SUB_CLASSES(Simple128)
}

