//
// Created by dewe on 7/16/22.
//

#include "stopwatch.h"
#include "s2clientprotocol/sc2api.pb.h"
#include "protocol.h"
#include "remote_controller.h"


namespace sc2 {

    StarcraftProtocol::StarcraftProtocol(std::unique_ptr<web::web_sockets::client::websocket_client> socket,
                                         Port port):
    m_socket( std::move(socket) ),
    m_status(SC2APIProtocol::Status::launched ),
    m_port(port ),
    m_count(1){}

    sc_pb::Response StarcraftProtocol::send(sc_pb::Request& req){
        req.set_id(m_count++);
        sc_pb::Response res;
        auto _name = name(req);
        auto field = sc_pb::Request::descriptor()->field(0);
        auto _type = field->containing_oneof()->full_name();

        try{
            res = sendReq(req);
        } catch (websocket_exception const &exception) {
            throw ConnectionError(LOG_STREAM("Error during " << _name << ": " << exception.what()));
        }
        if( res.has_id() and res.id() != req.id()){
            throw ConnectionError(LOG_STREAM("Error during " << _name << ": Got a response with a different id"));
        }
        return res;
    }

    sc_pb::Response  StarcraftProtocol::sendReq(sc_pb::Request& req){
        write(req);
        return read();
    }

    StarcraftProtocol::~StarcraftProtocol() {
        m_socket->close().wait();
        m_status = SC2APIProtocol::Status::quit;
    }

    SC2APIProtocol::Response StarcraftProtocol::read() {
        TimePoint start;
        auto sc2_verbose_protocol = get_env("SC2_VERBOSE_PROTOCOL");

        if(not sc2_verbose_protocol.empty() ){
            log(LOG_STREAM("-------------- " << m_port << " Reading response --------------"));
            start = std::chrono::high_resolution_clock::now();
        }

        auto response = _read();
        if( not sc2_verbose_protocol.empty() ) {
            log(LOG_STREAM("-------------- [" << m_port << "]  Read " <<
                                              name(response) << " in " << elapsed(start) * 1000
                                              << " msec --------------\n" << packetStr(response)));
        }

        if( not response.has_status() ){
            throw ProtocolError("Got an incomplete response without a status.");
        }
        auto prev_status = m_status;
        m_status = sc_pb::Status(response.status());
        if(not response.error().empty()){
            throw ProtocolError(LOG_STREAM("Error in RPC response (likely a bug). "
                                           "Prev status: " << prev_status << " new status: "
                                                           << m_status << " error: \n" << response.error() ));
        }
        return response;
    }

    void StarcraftProtocol::write(sc_pb::Request& req){
        if( not get_env("SC2_VERBOSE_PROTOCOL").empty() ){
            log(LOG_STREAM("-------------- [" << m_port <<
            "] Writing request: " << name(req) << "--------------\n" << packetStr(req) ));
        }
        _write(req);
    }

    template<class RequestOrResponse>
    std::string StarcraftProtocol::packetStr(RequestOrResponse &req) {
        auto max_lines = std::stoi( get_env("SC2_VERBOSE_PROTOCOL") );
        auto packet_str = req.Utf8DebugString();
        if(max_lines <= 0){
            return packet_str;
        }
        packet_str.pop_back();
        auto lines = split(packet_str, "\n");
        auto line_count = lines.size();
        std::vector<std::string> new_lines(std::min<int>(line_count, max_lines + 1));
        std::transform(lines.begin(), lines.begin() + new_lines.size(), new_lines.begin(),
                       [](auto const& line){
            return line.substr(0, MAX_WIDTH);
        });

        if( line_count > max_lines + 1){
            new_lines.back() = LOG_STREAM("***** " << (line_count - max_lines) << " lines skipped *****");
        }

        SplitToken token("\n");
        return LOG_STREAM(new_lines);
    }

    void StarcraftProtocol::_write(sc_pb::Request& req) {

        std::vector<uint8_t> body(req.ByteSizeLong());
        {
            With w{sw("serialize_request")};
            req.SerializeToArray(body.data(), (int) body.size());
        }

        With w{sw("write_request")};
        concurrency::streams::producer_consumer_buffer<uint8_t> buf;
        websocket_outgoing_message _msg;
        buf.putn_nocopy(body.data(), body.size())
            .then([&](size_t length) {
                _msg.set_binary_message(buf.create_istream(), length);
                return m_socket->send(_msg);
            }).then([](pplx::task<void> const& t){
              try
              {
                  t.get();
              }
              catch(const websocket_exception& ex)
              {
                  std::string msg = ex.what();
                  std::cerr << msg << "\n";
                  if (boost::contains(msg, "Close")) {
                      std::cerr << "Connection already closed. SC2 probably crashed. "
                                   "Check the error log.";
                  }
                  throw;
              }
          }).wait();

    }

    sc_pb::Response StarcraftProtocol::_read(){
        concurrency::streams::istream stream;

        {
            With w{sw("read_response")};
            auto task = m_socket->receive().then([](web::web_sockets::client::websocket_incoming_message const& in_msg){
                return in_msg.body();
            });
            try
            {
                stream = task.get();
            }
            catch(const websocket_exception& ex)
            {
                std::cout << ex.what() << "\n";
            }
            task.wait();
        }

        With w{sw("parse_response")};
        SC2APIProtocol::Response resp;
        auto  buf = stream.streambuf();
        auto total = static_cast<int>( buf.size() );
        std::vector<uint8_t> blob(total);
        buf.getn(blob.data(), total);
        resp.ParseFromArray( blob.data() , total);
        return resp;
    }

    std::string StarcraftProtocol::name(sc_pb::Request const& req) {
        switch (req.request_case()) {
            case SC2APIProtocol::Request::kCreateGame:
                return req.create_game().GetTypeName();
            case SC2APIProtocol::Request::kJoinGame:
                return req.join_game().GetTypeName();
            case SC2APIProtocol::Request::kRestartGame:
                return req.restart_game().GetTypeName();
            case SC2APIProtocol::Request::kStartReplay:
                return req.start_replay().GetTypeName();
            case SC2APIProtocol::Request::kLeaveGame:
                return req.leave_game().GetTypeName();
            case SC2APIProtocol::Request::kQuickSave:
                return req.quick_save().GetTypeName();
            case SC2APIProtocol::Request::kQuickLoad:
                return req.quick_load().GetTypeName();
            case SC2APIProtocol::Request::kQuit:
                return req.quit().GetTypeName();
            case SC2APIProtocol::Request::kGameInfo:
                return req.game_info().GetTypeName();
            case SC2APIProtocol::Request::kObservation:
                return req.observation().GetTypeName();
            case SC2APIProtocol::Request::kAction:
                return req.action().GetTypeName();
            case SC2APIProtocol::Request::kObsAction:
                return req.obs_action().GetTypeName();
            case SC2APIProtocol::Request::kStep:
                return req.step().GetTypeName();
            case SC2APIProtocol::Request::kData:
                return req.data().GetTypeName();
            case SC2APIProtocol::Request::kQuery:
                return req.query().GetTypeName();
            case SC2APIProtocol::Request::kSaveReplay:
                return req.save_replay().GetTypeName();
            case SC2APIProtocol::Request::kMapCommand:
                return req.map_command().GetTypeName();
            case SC2APIProtocol::Request::kReplayInfo:
                return req.replay_info().GetTypeName();
            case SC2APIProtocol::Request::kAvailableMaps:
                return req.available_maps().GetTypeName();
            case SC2APIProtocol::Request::kSaveMap:
                return req.save_map().GetTypeName();
            case SC2APIProtocol::Request::kPing:
                return req.ping().GetTypeName();
            case SC2APIProtocol::Request::kDebug:
                return req.debug().GetTypeName();
            case SC2APIProtocol::Request::REQUEST_NOT_SET:
                return "None";
        }
    }

    std::string StarcraftProtocol::name(const sc_pb::Response &req) {
        switch (req.response_case()) {
            case SC2APIProtocol::Response::kCreateGame:
                return req.create_game().GetTypeName();
            case SC2APIProtocol::Response::kJoinGame:
                return req.join_game().GetTypeName();
            case SC2APIProtocol::Response::kRestartGame:
                return req.restart_game().GetTypeName();
            case SC2APIProtocol::Response::kStartReplay:
                return req.start_replay().GetTypeName();
            case SC2APIProtocol::Response::kLeaveGame:
                return req.leave_game().GetTypeName();
            case SC2APIProtocol::Response::kQuickSave:
                return req.quick_save().GetTypeName();
            case SC2APIProtocol::Response::kQuickLoad:
                return req.quick_load().GetTypeName();
            case SC2APIProtocol::Response::kQuit:
                return req.quit().GetTypeName();
            case SC2APIProtocol::Response::kGameInfo:
                return req.game_info().GetTypeName();
            case SC2APIProtocol::Response::kObservation:
                return req.observation().GetTypeName();
            case SC2APIProtocol::Response::kAction:
                return req.action().GetTypeName();
            case SC2APIProtocol::Response::kObsAction:
                return req.obs_action().GetTypeName();
            case SC2APIProtocol::Response::kStep:
                return req.step().GetTypeName();
            case SC2APIProtocol::Response::kData:
                return req.data().GetTypeName();
            case SC2APIProtocol::Response::kQuery:
                return req.query().GetTypeName();
            case SC2APIProtocol::Response::kSaveReplay:
                return req.save_replay().GetTypeName();
            case SC2APIProtocol::Response::kReplayInfo:
                return req.replay_info().GetTypeName();
            case SC2APIProtocol::Response::kAvailableMaps:
                return req.available_maps().GetTypeName();
            case SC2APIProtocol::Response::kSaveMap:
                return req.save_map().GetTypeName();
            case SC2APIProtocol::Response::kMapCommand:
                return req.map_command().GetTypeName();
            case SC2APIProtocol::Response::kPing:
                return req.ping().GetTypeName();
            case SC2APIProtocol::Response::kDebug:
                return req.debug().GetTypeName();
            case SC2APIProtocol::Response::RESPONSE_NOT_SET:
                return "None";
        }
    }


    template std::string StarcraftProtocol::packetStr<sc_pb::Request>(sc_pb::Request& );
    template std::string StarcraftProtocol::packetStr<sc_pb::Response>(sc_pb::Response& );
}