#pragma once
//
// Created by dewe on 7/16/22.
//

#include <boost/process/child.hpp>
#include <utility>
#include "protocol.h"
#include "../maps/lib.h"
#include "cpprest/ws_client.h"
#include "static_data.h"


template<typename T>
concept HasErrorDetails = requires(T t) { t.error_details(); };

namespace sc2{

    struct ConnectError : std::exception {
        std::string msg;
        explicit ConnectError(std::string const &_msg) : msg( "ConnectError: " + _msg) {}
        [[nodiscard]] const char *what() const noexcept override {
            return msg.c_str();
        }
    };

    template<class ResponseT>
    struct RequestError : std::exception {
        std::string msg;
        ResponseT res;
        explicit RequestError(std::string const &description, ResponseT  _res) :
        msg( "RequestError: " + description), res(std::move(_res)) {}

        [[nodiscard]] const char *what() const noexcept override {
            return msg.c_str();
        }
    };

    class RemoteController {
        std::unique_ptr<StarcraftProtocol> m_client;
        std::optional<sc_pb::ResponseObservation> m_lastObs;

        std::unique_ptr<web::web_sockets::client::websocket_client> connect(std::string const& host,
                     unsigned short port,
                     boost::process::child* proc,
                     int timeout_seconds);

        template<class ResponseType>
        std::function<std::function<ResponseType()> (std::function<ResponseType()> const&) > decorateError() {
            return [](std::function<ResponseType()> const &func) -> std::function<ResponseType()> {
                auto res = func();
                return [res]() -> ResponseType {
                    if (res.has_error()) {
                        auto error_name = ResponseType::Error_Name(res.error());
                        auto desc = ResponseType::Error_descriptor()->FindValueByNumber(res.error());
                        auto enum_name = desc->full_name();
                        std::string details = "<none>";
                        if constexpr ( HasErrorDetails<ResponseType> ){
                            details = res.error_details();
                        }
                        throw RequestError(LOG_STREAM(enum_name << "." << error_name << ": " << details), res);
                    }
                };
            };
        }

        template<class Ret>
        std::function< std::function<Ret()> (std::function<Ret()> const& ) >
                validStatus(std::vector<SC2APIProtocol::Status> const& valid ){
            return [this, &valid]( std::function<Ret()> const& func) -> std::function<Ret()> {
                auto _valid_status =  [&valid, this, &func]() -> Ret {
                    if( std::ranges::find(valid, this->status() ) == valid.end() ){
                        throw ProtocolError(
                                LOG_STREAM(boost::typeindex::type_id<Ret>().name() << "()" <<
                                "called while in state: " << this->status() << ", valid: " << valid ));
                    }
                    return func();
                };
                return _valid_status;
            };
        }

        template<class Ret>
        std::function< std::function<Ret()> (std::function<Ret()> const& ) >
        skipStatus(std::vector<SC2APIProtocol::Status> const& skipped ){
            return [this, &skipped]( std::function<Ret()> const& func) -> std::function<Ret()> {
                auto _skip_status =  [&skipped, this, &func]() -> Ret {
                    if (std::ranges::find(skipped, this->status()) == skipped.end()) {
                        return func();
                    }else{
                        return {};
                    }
                };
                return _skip_status;
            };
        }

        template<class Ret>
        std::function< std::function<Ret()> (std::function<Ret()> const& ) >
        catchGameEnd(){
            return [this]( std::function<Ret()> const& func) -> std::function<Ret()> {
                auto prevStatus = status();
                return [prevStatus, &func]() -> Ret{
                    try{
                        return func();
                    } catch (ProtocolError const& protocolError) {
                        if( prevStatus == SC2APIProtocol::Status::in_game
                            and boost::contains("Game has already ended", protocolError.msg)){
                            std::cerr << "Received a 'Game has already ended' error from SC2 whilst status "
                                         "in_game. Suppressing the exception, returning None.\n";
                        }
                        throw;
                    }
                };
            };
        }

        template<typename Typename, typename Callable>
        Typename ValidateStatusCheckErrorRecordFunction(std::string const& fnc_name,
                                                    Callable && _func,
                                                    std::vector<sc_pb::Status> const& valid_statuses){
            return validStatus<Typename>(valid_statuses)(
                    decorateError<Typename>()(
                            sw.decorate<Typename>(fnc_name)(_func) ) )();
        }

        template<typename Typename, typename Callable>
        Typename CheckErrorRecordFunction(std::string const& fnc_name,
                                                        Callable && _func){
            return decorateError<Typename>()(sw.decorate<Typename>(fnc_name)(_func) )();
        }

        template<typename Typename, typename Callable>
        Typename ValidateStatusRecordFunction(std::string const& fnc_name,
                                                        Callable && _func,
                                                        std::vector<sc_pb::Status> const& valid_statuses){
            return validStatus<Typename>(valid_statuses)(sw.decorate<Typename>(fnc_name)(_func) )();
        }

        template<typename Typename, typename Callable>
        Typename SkipStatusRecordFunction(std::string const& fnc_name,
                                              Callable && _func,
                                              std::vector<sc_pb::Status> const& statuses){
            return skipStatus<Typename>(statuses)(sw.decorate<Typename>(fnc_name)(_func) )();
        }

        template<typename Typename, typename Callable>
        Typename SkipStatusValidateStatusRecordFunction(std::string const& fnc_name,
                                          Callable && _func,
                                          std::vector<sc_pb::Status> const& skip_statuses,
                                          std::vector<sc_pb::Status> const& valid_statuses){
            return skipStatus<Typename>(skip_statuses)(
                    validStatus<Typename>(valid_statuses)(
                            sw.decorate<Typename>(fnc_name)(_func) ) )();
        }

        template<typename Typename, typename Callable>
        Typename SkipStatusValidateStatusCatchGameEndRecordFunction(std::string const& fnc_name,
                                                        Callable && _func,
                                                        std::vector<sc_pb::Status> const& skip_statuses,
                                                        std::vector<sc_pb::Status> const& valid_statuses){
            return skipStatus<Typename>(skip_statuses)(
                    validStatus<Typename>(valid_statuses)(
                            catchGameEnd<Typename>()(
                                    sw.decorate<Typename>(fnc_name)(_func) ) ) )();
        }

        template<typename Typename, typename Callable>
        Typename ValidateStatusCatchGameEndRecordFunction(std::string const& fnc_name,
                                                                    Callable && _func,
                                                                    std::vector<sc_pb::Status> const& valid_statuses){
            return validStatus<Typename>(valid_statuses)(
                            catchGameEnd<Typename>()(
                                    sw.decorate<Typename>(fnc_name)(_func) ) )();
        }

        void close(){
            m_client.reset(nullptr);
        }

    public:
        RemoteController(std::string const& host,
                         unsigned short port,
                         boost::process::child* proc= nullptr,
                         std::optional<int> timeout_seconds=std::nullopt);

        inline bool statusEnded() { return m_client->status() == sc_pb::Status::ended; }

        [[nodiscard]] inline auto createGame(sc_pb::Request& create_req) {
            return ValidateStatusCheckErrorRecordFunction<sc_pb::ResponseCreateGame>(
                    "create_game",
                    [this, &create_req]() { return m_client->send(create_req).create_game(); },
                    {SC2APIProtocol::launched, SC2APIProtocol::ended, SC2APIProtocol::in_game,
                     SC2APIProtocol::in_replay});
        }

        sc_pb::ResponseSaveMap saveMap(std::filesystem::path const& mapPath,
                                       std::string const& map_data);

        [[nodiscard]] inline auto joinGame(sc_pb::Request& join_req){
            return ValidateStatusCheckErrorRecordFunction<sc_pb::ResponseJoinGame>(
                    "join_game",
                    [this, &join_req]() { return m_client->send(join_req).join_game(); },
                    {sc_pb::launched, sc_pb::init_game});
        }

        [[nodiscard]] inline auto restart(){
            return ValidateStatusCheckErrorRecordFunction<sc_pb::ResponseRestartGame>(
                    "restart_game",
                    [this]() { sc_pb::Request req; req.restart_game(); return m_client->send(req).restart_game(); },
                    {sc_pb::ended, sc_pb::in_game});
        }

        [[nodiscard]] inline auto startReplay(sc_pb::Request& req_start_replay){
            return ValidateStatusCheckErrorRecordFunction<sc_pb::ResponseStartReplay>(
                    "start_replay",
                    [this, &req_start_replay]() { return m_client->send(req_start_replay).start_replay(); },
                    {sc_pb::launched, sc_pb::ended, sc_pb::in_game, sc_pb::in_replay});
        }

        [[nodiscard]] inline auto  gameInfo(){
            return ValidateStatusRecordFunction<sc_pb::ResponseGameInfo>(
                    "game_info",
                    [this]() { sc_pb::Request req; req.game_info(); return m_client->send(req).game_info(); },
                    {sc_pb::in_replay, sc_pb::in_game});
        }

        [[nodiscard]] inline auto dataRaw(sc_pb::Request& req_data){
            return ValidateStatusRecordFunction<sc_pb::ResponseData>(
                    "data_raw",
                    [this, &req_data]() { return m_client->send(req_data).data(); },
                    {sc_pb::in_game, sc_pb::in_replay});
        }

        [[nodiscard]] inline auto data() {
            sc_pb::Request req; auto data_req = req.mutable_data();
            data_req->set_ability_id(true); data_req->set_unit_type_id(true); data_req->set_upgrade_id(true);
            data_req->set_buff_id(true); data_req->set_effect_id(true);
            return StaticData{dataRaw(req)};
        };

        sc_pb::ResponseObservation observe(bool disable_fog=false, int target_game_loop=0);

        std::vector< std::string > available_maps();

        [[nodiscard]] inline auto step(int count=1){
            return ValidateStatusCatchGameEndRecordFunction<sc_pb::ResponseStep>(
                    "step",
                    [this, count]() { sc_pb::Request req; req.mutable_step()->set_count(count); return m_client->send(req).step(); },
                    {sc_pb::in_game, sc_pb::in_replay});
        }

        sc_pb::ResponseAction actions(sc_pb::Request& action);

        sc_pb::ResponseAction act(sc_pb::Request& action){
            if(action.has_action() and action.action().GetMetadata().descriptor->field_count() > 0){
                return actions(action);
            }
        }

        sc_pb::ResponseObserverAction observerAction(sc_pb::Request& observer_action);

        sc_pb::ResponseObserverAction observerAct(sc_pb::Request& observer_action){
            if(observer_action.has_action() and observer_action.action().GetMetadata().descriptor->field_count() > 0){
                return observerAction(observer_action);
            }
        }

        sc_pb::ResponseAction chat(std::optional<std::string> const& message,
                                   sc_pb::ActionChat_Channel const& channel
                                   =sc_pb::ActionChat_Channel::ActionChat_Channel_Broadcast){
            if(message){
                sc_pb::Request req;
                auto action_chat = req.mutable_action()->add_actions();
                action_chat->mutable_action_chat()->set_channel(channel);
                action_chat->mutable_action_chat()->set_message(*message);
                return act(req);
            }
        }

        sc_pb::ResponseLeaveGame leave(){
            return ValidateStatusRecordFunction<sc_pb::ResponseLeaveGame>(
                    "leave",
                    [this]() { sc_pb::Request req; req.leave_game(); return m_client->send(req).leave_game(); },
                    {sc_pb::in_game, sc_pb::ended});
        }

        std::string saveReplay(){
            return ValidateStatusRecordFunction<sc_pb::ResponseSaveReplay>(
                    "save_replay",
                    [this]() { sc_pb::Request req; req.save_replay(); return m_client->send(req).save_replay(); },
                    {sc_pb::in_game, sc_pb::in_replay, sc_pb::ended}).data();
        }

        sc_pb::ResponseDebug debug(sc_pb::DebugCommand& cmd){
            return ValidateStatusRecordFunction<sc_pb::ResponseDebug>(
                    "debug",
                    [this, &cmd]() {
                        sc_pb::Request req;
                        req.mutable_debug()->mutable_debug()->AddAllocated(&cmd);
                        return m_client->send(req).debug(); },
                    {sc_pb::in_game, sc_pb::ended});
        }

        sc_pb::ResponseDebug debug( std::vector<sc_pb::DebugCommand>& cmds){
            return ValidateStatusRecordFunction<sc_pb::ResponseDebug>(
                    "debug",
                    [this, &cmds]() {
                        sc_pb::Request req;
                        auto dbg = req.mutable_debug();
                        for(auto& cmd: cmds)
                            dbg->mutable_debug()->AddAllocated(&cmd);
                        return m_client->send(req).debug(); },
                    {sc_pb::in_game, sc_pb::ended});
        }

        sc_pb::ResponseQuery query( sc_pb::Request& req){
            return ValidateStatusRecordFunction<sc_pb::ResponseQuery>(
                    "query",
                    [this, &req]() { return m_client->send(req).query(); },
                    {sc_pb::in_game, sc_pb::in_replay});
        }

        inline void quit() {
            auto result = SkipStatusRecordFunction<sc_pb::ResponsePing>(
                    "quit",
                    [this]() -> sc_pb::ResponsePing {
                        try {
                            sc_pb::Request req;
                            req.set_id(999999999);
                            req.mutable_quit();
                            m_client->write(req);
                            return {};
                        } catch (ConnectionError const &err) {}
                    }, {sc_pb::quit});
            close();
        }

        [[nodiscard]] inline auto replayInfo(std::string const& replay_data){
            return CheckErrorRecordFunction<sc_pb::ResponseReplayInfo>(
                    "replay_info",
                    [this, &replay_data]() {
                        sc_pb::Request req;
                        req.mutable_replay_info()->set_replay_data(replay_data);
                        return m_client->send(req).replay_info(); });
        }

        SC2APIProtocol::ResponsePing ping();

        [[nodiscard]] inline SC2APIProtocol::Status status() const { return m_client->status(); }

    };
}
