#pragma once
//
// Created by dewe on 7/14/22.
//
#include "memory"
#include "string"
#include "vector"
#include "unordered_map"
#include "custom/sc2/maps/lib.h"
#include "custom/sc2/maps/mini_games.h"
#include "custom/sc2/run_configs/lib.h"


namespace sc2{

    const std::unordered_map<std::string, std::shared_ptr<Map> > Maps{
            {"MoveToBeacon", std::static_pointer_cast<Map>(std::make_shared<MoveToBeacon>()) },
            {"FindAndDefeatZerglings", std::static_pointer_cast<Map>(std::make_shared<FindAndDefeatZerglings>()) }
    };

    namespace map{
        const std::unordered_map<std::string, std::vector< std::string > > __subclasses__{
                {"MiniGame", {"MoveToBeacon", "FindAndDefeatZerglings"} }
        };
    }

    namespace rc{
        const std::unordered_map<std::string, std::vector< std::string > > __subclasses__{
                {"RunConfig", {"MoveToBeacon", "FindAndDefeatZerglings"} }
        };
    }

    const std::unordered_map<std::string, std::shared_ptr<RunConfig> > RunConfigs{
    };

}
