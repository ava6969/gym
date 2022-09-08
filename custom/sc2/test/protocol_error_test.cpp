//
// Created by dewe on 9/6/22.
//


#include "catch2/catch.hpp"
#include "../run_configs/lib.h"
#include "../lib/sc_process.h"
#include "sc2/registry.h"

namespace sc2{
    struct StopwatchProfile{

        StopwatchProfile(){
            sw.clear();
            sw.enable();
        }

        ~StopwatchProfile(){
            auto s = sw.str();
            if(not s.empty())
                std::cerr << s << "\n";
            sw.disable();
        }
    };
}


TEST_CASE("TestProtocolError"){


    setenv("SC2_TIMEOUT", "10", 1);
    setenv("SC2_VERBOSE_PROTOCOL", "20", 1);

    SECTION("test_error"){
        sc2::StopwatchProfile profile;

        auto proc = sc2::RunConfig("latest").start(false);
        auto& controller = proc->controller();

        sc_pb::Request req;
        req.mutable_create_game();
        REQUIRE_THROWS_AS( controller.createGame(req), sc2::RequestError<sc_pb::ResponseCreateGame>);

        req.mutable_join_game();
        REQUIRE_THROWS_AS(controller.joinGame(req), sc2::ProtocolError);

    }

    SECTION("test_replay_a_replay"){
        sc2::StopwatchProfile profile;
        auto run_config = sc2::RunConfig("latest");
        auto proc = run_config.start(false);
        auto& controller = proc->controller();

        auto map_inst = sc2::Maps.at("Flat64");
        auto map_data = map_inst->data(run_config);

        // Play a quick game to generate a replay.
        sc_pb::Request create_req, join_req, start_replay_req;
        auto create = create_req.mutable_create_game();
        create->mutable_local_map()->set_map_path(map_inst->path().value());
        create->mutable_local_map()->set_map_data(map_data);
        create->add_player_setup()->set_type(sc_pb::Participant);

        auto ai =create->add_player_setup();
        ai->set_type(sc_pb::Computer);
        ai->set_race(sc_pb::Terran);
        ai->set_difficulty(sc_pb::VeryEasy);

        auto join = join_req.join_game();
        join.set_race(SC2APIProtocol::Terran);
        join.mutable_options()->set_raw(true);

        controller.createGame(create_req);
        controller.joinGame(join_req);
        controller.step(100);
        auto obs = controller.observe();
        auto replay_data = controller.saveReplay();

        // Run through the replay verifying that it finishes but wasn't recording
        // a replay
        auto start_replay = start_replay_req.mutable_start_replay();
        start_replay->set_replay_data(replay_data);
        start_replay->set_map_data(map_data);
        start_replay->mutable_options()->CopyFrom( join.options() );
        start_replay->set_observed_player_id(1);

        controller.startReplay(start_replay_req);
        controller.step(1000);
        auto obs2 = controller.observe();

        REQUIRE(obs.observation().game_loop() == obs2.observation().game_loop());
        REQUIRE_THROWS_AS(controller.saveReplay(), sc2::ProtocolError);

    }

}