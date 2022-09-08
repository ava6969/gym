#pragma once
//
// Created by dewe on 7/14/22.
//
#include "memory"
#include "string"
#include "vector"
#include "unordered_map"
#include "maps/lib.h"
#include "maps/mini_games.h"
#include "run_configs/lib.h"
#include "sc2/maps/melee.h"


namespace sc2{

#define MAKE_ITEM(arg) {#arg, std::static_pointer_cast<Map>(std::make_shared<arg>()) }

    const std::unordered_map<std::string, std::shared_ptr<Map> > Maps{
            MAKE_ITEM(MoveToBeacon),  MAKE_ITEM(FindAndDefeatZerglings),  MAKE_ITEM(Simple64),  MAKE_ITEM(Simple96)
    };

    namespace map{
        const std::unordered_map<std::string, std::vector< std::string > > __subclasses__{
                {"MiniGame", {"MoveToBeacon", "FindAndDefeatZerglings"} },
                {"Melee", {"Simple64", "Simple64"} }
        };
    }

}
