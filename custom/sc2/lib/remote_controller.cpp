//
// Created by dewe on 7/16/22.
//

#include "protocol.h"
#include "stopwatch.h"
#include "s2clientprotocol/sc2api.pb.h"
#include "remote_controller.h"

using namespace web;
using namespace web::websockets::client;

namespace sc2 {
    RemoteController::RemoteController(const std::string &host, unsigned short port,
                                       boost::process::child *proc,
                                       std::optional<int> timeout_seconds) {

        if (not timeout_seconds) {
            if (auto sc2_timeout = getenv("SC2_TIMEOUT")) {
                timeout_seconds = std::stoi(sc2_timeout);
            } else {
                throw std::runtime_error("timeout_seconds must be set or $ENV{SC2_TIMEOUT}");
            }
        }

        m_client = std::make_unique<StarcraftProtocol>(connect(host, port, proc, *timeout_seconds), port);
        m_lastObs = std::nullopt;
        ping();
    }

    std::unique_ptr<websocket_client> RemoteController::connect(std::string const &host_ref,
                                                                unsigned short port,
                                                                boost::process::child *proc,
                                                                int timeout_seconds) {

        return sw.decorate<std::unique_ptr<websocket_client> >("connect")([host_ref, port, timeout_seconds, proc]() {
            auto host = host_ref;
            if (host.find(':') != std::string::npos and (not host.starts_with("["))) {
                host = LOG_STREAM("[" << host << "]");
            }

            auto port_str = std::to_string(port); // prevent stream from adding commas

            auto url = LOG_STREAM("ws://" << host << ":" << port_str << "/sc2api");
            auto was_running = false;

            for (int i = 0; i < timeout_seconds; i++) {
                auto is_running = proc and proc->running();
                was_running = was_running || is_running;
                if ((i >= int(timeout_seconds / 4) or was_running) and not is_running) {
                    std::cerr << "SC2 isn't running, so bailing early on the websocket connection.\n";
                    break;
                }
                std::cerr << "Connecting to: " << url << ", attempt: " << i << ", running: " << is_running << "\n";

                try {
                    auto client = std::make_unique<websocket_client>();
                    client->connect(U(url)).then([]() {
                        std::cerr << "WebsocketClient: Connected to client successfully\n";
                    }).wait();
                    return std::move(client);
                } catch (websocket_exception const &exception) {
                    auto code = exception.error_code().value();
                    if (code != 2) {
                        throw ConnectError("Connection rejected. Is something else connected?");
                    }else{
                        std::cerr << exception.error_code().message() << "\n";
                    }
//                    sleep(2);
                }
            }
            throw ConnectError("Failed to connect to the SC2 websocket. Is it up?");
        })();
    }

    SC2APIProtocol::ResponsePing RemoteController::ping() {
        SC2APIProtocol::Request req;
        req.mutable_ping();
        return sw.decorate<SC2APIProtocol::Response>("ping")([this, &req]() {
            return m_client->send(req);
        })().ping();
    }

    std::vector<std::string> RemoteController::available_maps() {
        SC2APIProtocol::Request req;
        req.available_maps();
        auto response = m_client->send(req).available_maps();
        std::vector<std::string> result(response.battlenet_map_names_size());
        std::ranges::transform(response.battlenet_map_names(), result.begin(), [](auto const &n) { return n; });
        return result;
    }

    sc_pb::ResponseSaveMap RemoteController::saveMap(std::filesystem::path const &map_path,
                                                     std::string const &map_data) {
        SC2APIProtocol::Request req;
        auto req_save_map = req.mutable_save_map();
        req_save_map->set_map_data(map_data);
        req_save_map->set_map_path(map_path.string());

        return ValidateStatusCheckErrorRecordFunction<sc_pb::ResponseSaveMap>(
                "save_map",
                [this, &req]() { return m_client->send(req).save_map(); },
                {SC2APIProtocol::launched, SC2APIProtocol::init_game});
    }

    sc_pb::ResponseObservation RemoteController::observe(bool disable_fog, int target_game_loop) {
        return ValidateStatusRecordFunction<sc_pb::ResponseObservation>(
                "observe",
                [this, disable_fog, target_game_loop]() {
                    sc_pb::Request req;

                    auto obs_req = req.mutable_observation();
                    obs_req->set_game_loop(target_game_loop);
                    obs_req->set_disable_fog(disable_fog);

                    auto obs = m_client->send(
                            req).mutable_observation();
                    if (obs->observation().game_loop() ==
                        UINT_MAX) {
                        std::cerr
                                << "Received stub observation.";

                        if (obs->player_result().empty()) {
                            throw std::out_of_range(
                                    "Expect a player result in a stub observation");
                        } else if (not m_lastObs) {
                            throw std::runtime_error(
                                    "Received stub observation with no previous obs");
                        }

                        auto new_obs = m_lastObs.value();
                        new_obs.clear_actions();
                        new_obs.mutable_actions()->MergeFrom(
                                obs->actions());
                        new_obs.mutable_player_result()->MergeFrom(
                                obs->player_result());
                        obs->CopyFrom(new_obs);
                        m_lastObs = std::nullopt;
                    } else {
                        m_lastObs = *obs;
                    }

                    if (not get_env(
                            "SC2_LOG_ACTIONS").empty() and
                        not obs->actions().empty()) {
                        std::cerr
                                << "<<<<<<<<<<<<<<<<<<<<< Executed actions <<<<<<<<<<<<<<<<<<<<<\n";
                        for (auto const &action: obs->actions()) {
                            std::cerr
                                    << action.Utf8DebugString()
                                    << "\n";
                        }
                    }

                    return *obs;
                }, {sc_pb::in_game, sc_pb::in_replay,
                    sc_pb::ended});
    }

    sc_pb::ResponseAction RemoteController::actions(sc_pb::Request &req_action) {
        return SkipStatusValidateStatusCatchGameEndRecordFunction<sc_pb::ResponseAction>(
                "actions",
                [this, &req_action]() {
                    if (not get_env("SC2_LOG_ACTIONS").empty() and req_action.has_action()) {
                        std::cerr << "<<<<<<<<<<<<<<<<<<<<< Sending actions <<<<<<<<<<<<<<<<<<<<<\n";
                        for (auto const &action: req_action.action().actions()) {
                            std::cerr << action.Utf8DebugString() << "\n";
                        }
                    }
                    return m_client->send(req_action).action();
                }, {sc_pb::in_replay}, {sc_pb::in_game});
    }

    sc_pb::ResponseObserverAction RemoteController::observerAction(sc_pb::Request &req_action) {
        return SkipStatusValidateStatusRecordFunction<sc_pb::ResponseObserverAction>(
                "observer_actions",
                [this, &req_action]() {
                    if (not get_env("SC2_LOG_ACTIONS").empty() and req_action.has_action() ) {
                        std::cerr << "<<<<<<<<<<<<<<<<<<<<< Sending actions <<<<<<<<<<<<<<<<<<<<<\n";
                        for (auto const &action: req_action.action().actions()) {
                            std::cerr << action.Utf8DebugString() << "\n";
                        }
                    }
                    return m_client->send(req_action).obs_action();
                }, {sc_pb::in_game}, {sc_pb::in_replay});
    }
}

