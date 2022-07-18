#pragma once
//
// Created by dewe on 7/16/22.
//

#include "stdexcept"
#include "../run_configs/lib.h"
#include "point.h"
#include "remote_controller.h"
#include "boost/process.hpp"


namespace sc2 {

    struct SC2LaunchError : std::exception {
        std::string msg;
        SC2LaunchError(std::string const &_msg) : msg( "SC2LaunchError: " + _msg) {}
        const char *what() const noexcept override {
            return msg.c_str();
        }
    };

    class StarcraftProcess {

    public:
        template<typename ... Kwargs>
        StarcraftProcess(
                sc2::RunConfig const &run_config,
                std::filesystem::path const &exec_path,
                sc2::Version const &version,
                bool full_screen,
                std::unordered_map<std::string, std::any> extra_args,
                bool verbose,
                std::string const &host,
                unsigned short port,
                bool connect,
                int timeout,
                Point window_size, Point window_loc, Kwargs ...);

        ~StarcraftProcess();

        inline auto controller() const { return _controller; }

    private:
        RemoteController _controller;
        boost::process::child proc;
    };




}