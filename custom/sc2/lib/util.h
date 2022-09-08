#pragma once
//
// Created by dewe on 9/4/22.
//
#include "chrono"
#include "boost/algorithm/string.hpp"
#include <boost/process/search_path.hpp>
#include "boost/process.hpp"
#include "filesystem"
#include "string"
#include "sstream"
#include <google/protobuf/repeated_field.h>
#include <optional>

#define LOG_STREAM(_stream_) \
    [&](){std::stringstream ss; ss << _stream_; return ss.str(); }()

struct SplitToken{
    inline static std::string TOKEN =",";
    inline static std::mutex mtx;
    explicit SplitToken(std::string const& token){ mtx.lock(); TOKEN=token; }
    ~SplitToken() { TOKEN = ","; mtx.unlock(); }
} ;

using Port = unsigned short;
using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

struct PORTS{
    static inline std::unordered_set<Port> FREE, OWNED, RANDOM, CONTIGUOUS;
};

const std::vector<std::pair<int, int>> PROTOS{ {SOCK_STREAM, IPPROTO_TCP}, {SOCK_DGRAM, IPPROTO_UDP}};

template<typename Callable>
using ReturnType = typename decltype(std::function{std::declval<Callable>()})::result_type;

namespace sc2 {

    struct NoFreePortFoundError : std::exception{};

    static inline auto elapsed(TimePoint const &start) {
        return std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count();
    }

    static inline std::string upper(std::string const& v){
        std::string res = v;
        std::ranges::transform(v, res.begin(), toupper);
        return res;
    }

    static inline std::string lower(std::string const& v){
        std::string res = v;
        std::ranges::transform(v, res.begin(), tolower);
        return res;
    }

    static inline std::vector <std::string> split(std::string const &x, std::string const &token) {
        std::vector <std::string> sub_result;
        boost::split(sub_result, x, boost::is_any_of(token));
        return sub_result;
    }

    template<class Streamable>
    std::ostream& operator<<(std::ostream& os, std::vector<Streamable> const& s){
        std::ranges::copy(s, std::ostream_iterator<Streamable>(os, SplitToken::TOKEN.c_str()));
        return os;
    }

    static std::ostream& operator<<(std::ostream& os, google::protobuf::RepeatedPtrField<std::string> const& s){
        std::ranges::copy(s, std::ostream_iterator<std::string>(os, "\n"));
        return os;
    }

    static inline std::string get_env(std::string const& key){
        if( auto result = std::getenv(key.c_str()); result){
            return result;
        }
        return "";
    }

    static inline std::string get_env(std::string const& key, std::string const& default_v){
        if( auto result = std::getenv(key.c_str()); result){
            return result;
        }
        return default_v;
    }

    inline static void addReservedPort(Port port){
        PORTS::FREE.insert(port);
    }

    void returnPort(Port port);

    template<class Container>
    void returnPorts(Container const& ports){
        for(auto const& port: ports){
            if(auto it = PORTS::CONTIGUOUS.find(port); it != PORTS::CONTIGUOUS.end()){
                PORTS::CONTIGUOUS.erase(it);
            }else{
                returnPort(port);
            }
        }
    }

    bool isPortFree(Port port);

    std::optional<Port> bind(Port port, int socket_type, int socket_proto);

    Port pickUnusedPort();
    std::vector<Port> pickUnusedPorts(int num_ports, int retry_interval_secs=1, int retry_attempts=5);
    std::vector<Port> pickContiguousUnusedPort(int num_ports, int retry_interval_secs=1, int retry_attempts=5);

    struct With{
        std::shared_ptr<class StopWatchContext> ctx;
        explicit With( std::shared_ptr<StopWatchContext> const& _ctx);
        ~With();
    };

}