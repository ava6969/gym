//
// Created by dewe on 9/6/22.
//

#include "util.h"
#include "stopwatch.h"
#include "memory"
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "random"


namespace sc2{

    With::With( std::shared_ptr<StopWatchContext> const& _ctx):ctx(_ctx) {
        _ctx->enter();
    }

    With::~With() {
        ctx->exit();
    }

    int setipaddr(char *name, struct sockaddr *addr_ret, size_t addr_ret_size, int af){
        addrinfo hints{}, *res;
        int error;

        memset((void *) addr_ret, '\0', sizeof(*addr_ret));
        if (name[0] == '\0') {
            int siz;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = af;
            hints.ai_socktype = SOCK_DGRAM;
            hints.ai_flags = AI_PASSIVE;
            error = getaddrinfo(nullptr, "0", &hints, &res);
            if (error) {
                return -1;
            }
            siz = 4;
            if (res->ai_next) {
                freeaddrinfo(res);
                std::perror("wildcard resolved to multiple address");
                return -1;
            }
            if (res->ai_addrlen < addr_ret_size)
                addr_ret_size = res->ai_addrlen;
            memcpy(addr_ret, res->ai_addr, addr_ret_size);
            freeaddrinfo(res);
            return siz;
        }

        /* check for an IPv4 address */
        if (af == AF_INET || af == AF_UNSPEC) {
            auto *sin = (struct sockaddr_in *)addr_ret;
            memset(sin, 0, sizeof(*sin));
            if ((sin->sin_addr.s_addr = inet_addr(name)) != INADDR_NONE) {
                sin->sin_family = AF_INET;
                return 4;
            }
    }
    /* perform a name resolution */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = af;
    error = getaddrinfo(name, nullptr, &hints, &res);
    if (error) {
        return -1;
    }
    if (res->ai_addrlen < addr_ret_size)
        addr_ret_size = res->ai_addrlen;

    memcpy((char *) addr_ret, res->ai_addr, addr_ret_size);
    freeaddrinfo(res);
    return 4;
}

    int getsockaddrarg(int family, Port port, sockaddr *addr_ret, std::string& host, int& len_ret){
        sockaddr_in* addr;
        addr= reinterpret_cast<sockaddr_in*>(addr_ret);
        if( setipaddr(host.data(), (struct sockaddr *)addr, sizeof(*addr),  AF_INET) < 0)
            return 0;
        addr->sin_family = AF_INET;
        addr->sin_port = htons(port);
        len_ret = sizeof(*addr);
        return 1;
    }

    std::optional<Port> bind(Port port, int socket_type, int socket_proto) {

        bool got_socket = false;
        auto sock = socket(AF_INET, socket_type, socket_proto);
        if (sock != -1) {
            got_socket = true;

            int opt_val = 1;
            setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

            sockaddr addrbuf{};
            int addrlen{};
            std::string host;

            bool failed = false;

            if (not getsockaddrarg(AF_INET, static_cast<unsigned short>(port), &addrbuf, host, addrlen)) {
                failed = true;
            } else {

                if (bind(sock, &addrbuf, addrlen) != -1) {
                    if (socket_type == SOCK_STREAM) {
                        listen(sock, 1);
                    }

                    sockaddr_in addr{};
                    socklen_t _len = sizeof(addr);
                    if (getsockname(sock, (struct sockaddr *) &addr, &_len) == 0) {
                        port = addr.sin_port;
                    } else {
                        failed = true;
                    }
                } else {
                    std::cerr << std::strerror(errno) << '\n';
                    if (errno == EADDRINUSE) {
                        /* address already in use */
                        return std::nullopt;
                    }
                    failed = true;
                }
            }
            close(sock);
            if (failed) {
                return std::nullopt;
            }

        }

        return got_socket ? std::make_optional<Port>(port) : std::nullopt;
    }

    void returnPort(Port port){
        if(auto it = PORTS::RANDOM.find(port); it != PORTS::RANDOM.end()){
            PORTS::RANDOM.erase(it);
        }else if(it = PORTS::OWNED.find(port); it != PORTS::OWNED.end()){
            PORTS::OWNED.erase(it);
        }else if(PORTS::FREE.contains(port)){
            std::cerr << "Returning a port that was already returned: " << port << "\n";
        }else{
            std::cerr << "Returning a port that wasn't given by portpicker: " << port << "\n";
        }
    }

    bool isPortFree(Port port){
        return bind(port, PROTOS[0].first, PROTOS[0].second) and bind(port, PROTOS[1].first, PROTOS[1].second);
    }

    Port pickUnusedPort(){
        std::optional<int> port{};

        if(not PORTS::FREE.empty()){
            port = PORTS::FREE.extract(PORTS::FREE.begin()).value();
            PORTS::OWNED.insert(*port);
            return *port;
        }

        for(int i = 0; i < 10; i++){
            port = bind(0, PROTOS[0].first, PROTOS[0].second);

            if( port
                and (not PORTS::RANDOM.contains(*port))
                and (bind(*port, PROTOS[1].first, PROTOS[1].second))){
                PORTS::RANDOM.insert(*port);
                return port.value();
            }
        }

        std::random_device dev;
        std::seed_seq seeds{dev(), dev(), dev()};
        std::mt19937 gen(seeds);

        std::uniform_int_distribution<int> dist(15000, 25000);
        for(int i = 0; i < 10; i++){
            port = dist(gen);
            auto port_val = *port;
            if(not PORTS::RANDOM.contains(port_val)){
                if(isPortFree(port_val)){
                    PORTS::RANDOM.insert(port_val);
                    return port_val;
                }
            }
        }
        throw NoFreePortFoundError();
    }

    std::vector<Port> pickUnusedPorts(int num_ports, int retry_interval_secs, int retry_attempts){
        std::unordered_set<Port> ports;
        if(num_ports <= 0){
            throw std::runtime_error(LOG_STREAM("Number of ports, must be >= 1, got: " << num_ports));
        }

        for(int i = 0; i < retry_attempts; i++){
          for(int j = 0; j < (num_ports-ports.size()); j++){
              ports.insert(pickUnusedPort());
              if( ports.size() == num_ports)
                  return {ports.begin(), ports.end()};
          }
            std::this_thread::sleep_for(std::chrono::seconds(retry_interval_secs));
        }

        returnPorts(ports);
        throw std::runtime_error(LOG_STREAM("Unable to obtain " << num_ports << " unused ports."));
    }

    std::vector<Port> pickContiguousUnusedPort(int num_ports, int retry_interval_secs, int retry_attempts){
        std::vector<Port> ports(num_ports);
        if(num_ports <= 0){
            throw std::runtime_error(LOG_STREAM("Number of ports, must be >= 1, got: " << num_ports));
        }
        for(int i = 0; i < retry_attempts; i++){
            auto start_port = pickUnusedPort();
            std::iota(ports.begin(), ports.end(), start_port);
            if(std::ranges::all_of(ports, [](auto p){ return isPortFree(p); })){
                if(ports.size() > 1)
                    PORTS::CONTIGUOUS.insert(ports.begin() + 1, ports.end() );
                return ports;
            }else{
                returnPort(start_port);
            }
            std::this_thread::sleep_for(std::chrono::seconds(retry_interval_secs));
        }
        throw std::runtime_error(LOG_STREAM("Unable to obtain " << num_ports << " contiguous unused ports."));
    }
}