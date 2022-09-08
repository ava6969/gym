#pragma once
//
// Created by dewe on 7/14/22.
//

#include "lib.h"


namespace sc2{
    struct MiniGame : Map{
        MiniGame(){
            directory = "mini_games";
            download = "https://github.com/deepmind/pysc2#get-the-maps";
            m_players = 1;
            score_index = 0;
            game_steps_per_episode = 0;
            m_step_mul = 8;
        }
    };

    struct BuildMarines : MiniGame{
        BuildMarines(){
            filename = "BuildMarines";
        }
    };

    struct CollectMineralsAndGas : MiniGame{
        CollectMineralsAndGas(){
            filename = "CollectMineralsAndGas";
        }
    };

    struct CollectMineralShards : MiniGame{
        CollectMineralShards(){
            filename = "CollectMineralShards";
        }
    };

    struct DefeatRoaches : MiniGame{
        DefeatRoaches(){
            filename = "DefeatRoaches";
        }
    };

    struct DefeatZerglingsAndBanelings : MiniGame{
        DefeatZerglingsAndBanelings(){
            filename = "DefeatZerglingsAndBanelings";
        }
    };

    struct FindAndDefeatZerglings : MiniGame{
        FindAndDefeatZerglings(){
            filename = "FindAndDefeatZerglings";
        }
    };

    struct MoveToBeacon : MiniGame{
        MoveToBeacon(){
            filename = "MoveToBeacon";
        }
    };
}
