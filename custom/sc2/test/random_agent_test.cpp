//
// Created by dewe on 9/4/22.
//

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include "sc2/env/sc2_env.h"

TEST_CASE("random agent"){

    sc2::AgentInterfaceFormat::Option aif;

    SECTION(" Test Features"){
        aif.feature_dimensions = sc2::Dimensions().screen(84).minimap(64);
    }

    SECTION(" Test RGB"){
        aif.rgb_dimensions = sc2::Dimensions().screen(128).minimap(64);
    }

    SECTION(" Test All"){
        aif.feature_dimensions = sc2::Dimensions().screen(84).minimap(64);
        aif.rgb_dimensions = sc2::Dimensions().screen(128).minimap(64);
        aif.action_space = sc2::ActionSpace::Features;
        aif.use_unit_counts = true;
        aif.use_feature_units= true;
    }

    auto steps = 250;
    auto stepMul = 8;

    sc2::SC2Env::Option opt;
    opt.map_name = {"Simple64", "Simple96"};
    std::vector<sc2::PlayerPtr> players;
    players.emplace_back(
            std::make_unique<sc2::AgentImpl>( std::vector<sc_pb::Race>{sc_pb::Race::Random, sc_pb::Race::Terran} ));
    players.emplace_back(
            std::make_unique<sc2::BotImpl>(std::vector<sc_pb::Race>{sc_pb::Race::Zerg, sc_pb::Race::Protoss},
                                                   sc_pb::Difficulty::Easy,
                                                   std::vector<sc_pb::AIBuild>{sc_pb::AIBuild::Rush,
                                                                               sc_pb::AIBuild::Timing}));
    opt.agent_interface_format = {sc2::AgentInterfaceFormat(aif)};
    opt.step_mul = stepMul;
    opt.game_steps_per_episode = steps * int(stepMul/3);

//    auto agent = sc2::RandomAgent();
    sc2::SC2Env env(opt, std::move(players));

//    REQUIRE(agent.steps() == steps);
}