#pragma once
//
// Created by dewe on 7/16/22.
//


#include "cpprest/ws_client.h"
#include "util.h"
#include "s2clientprotocol/sc2api.pb.h"
#include "barrier"
#include "stopwatch.h"
#include <cpprest/producerconsumerstream.h>

using namespace web;
using namespace web::websockets::client;
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace concurrency::streams;       // Asynchronous streams
namespace sc_pb = SC2APIProtocol;

namespace sc2 {


    const int MAX_WIDTH = getenv("COLUMNS") ? std::stoi(getenv("COLUMNS")) : 200;

    struct ConnectionError : std::exception {
        std::string msg;
        explicit ConnectionError(std::string const &_msg) : msg( "ConnectionError: " + _msg) {}
        [[nodiscard]] const char *what() const noexcept override {
            return msg.c_str();
        }
    };

    struct ProtocolError : std::exception {
        std::string msg;
        explicit ProtocolError(std::string const &_msg) : msg( "ProtocolError: " + _msg) {}
        [[nodiscard]] const char *what() const noexcept override {
            return msg.c_str();
        }
    };

    class StarcraftProtocol {
        std::unique_ptr<web::web_sockets::client::websocket_client> m_socket;
        sc_pb::Status m_status;
        Port m_port;
        int m_count{};

        static inline void log(std::string const& msg){
            std::cerr << msg << "\n";
        }

        std::string name(sc_pb::Request const& req);
        std::string name(sc_pb::Response const& res);

    public:
        explicit StarcraftProtocol(std::unique_ptr<web::web_sockets::client::websocket_client> socket,
                                   Port port);

        ~StarcraftProtocol();

        sc_pb::Response send(sc_pb::Request& req);

        sc_pb::Response sendReq(sc_pb::Request& req);

        [[nodiscard]] inline auto status() const { return m_status; }

       void write(sc_pb::Request& req);

        template<class RequestOrResponse>
        static std::string packetStr(RequestOrResponse& req);

        void _write(sc_pb::Request& req);

        sc_pb::Response _read();

        sc_pb::Response read();

    };
}