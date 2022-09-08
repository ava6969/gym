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

    namespace fs = std::filesystem;

    struct SC2LaunchError : std::exception {
        std::string msg;
        explicit SC2LaunchError(std::string const &_msg) : msg( "SC2LaunchError: " + _msg) {}
        [[nodiscard]] const char *what() const noexcept override {
            return msg.c_str();
        }
    };

    class StarcraftProcess {

    public:
        StarcraftProcess(
                sc2::RunConfig const &run_config,
                std::filesystem::path const &exec_path,
                std::optional<sc2::Version> const &version,
                bool full_screen=false,
                std::vector<std::string> const& extra_args={},
                bool verbose=false,
                std::optional<std::string> const &host=std::nullopt,
                std::optional<unsigned short> const& port=std::nullopt,
                bool connect=true,
                std::optional<int> timeout=std::nullopt,
                Point window_size={640, 480},
                Point window_loc={50, 50},
                std::vector<unsigned short> const& extra_ports={});

        ~StarcraftProcess();

        [[nodiscard]] inline auto& controller() const { return *m_controller; }

        [[nodiscard]] inline auto pid() const  { return m_proc->id(); }

        [[nodiscard]] inline auto running() const  {
            return !get_env("SC2_PORT").empty() || (m_proc and m_proc->running());
        }

    private:
        std::unique_ptr<RemoteController> m_controller;
        std::unique_ptr<boost::process::child> m_proc;
        std::optional<std::filesystem::path> m_tmpDir;
        std::string m_host;
        std::optional<unsigned short> m_port;
        std::optional<Version> m_version;

        static void checkExist(std::filesystem::path const& path);

        template<class T>
        void addArg(std::stringstream& os, std::string const& k, T const& v){
            os << " -" << k << " " << v;
        }

        static void addArg(std::stringstream& os, std::string const& k){
            os << " -" << k;
        }

        static std::unique_ptr<boost::process::child> launch(RunConfig const& runConfig, std::stringstream const& args);

        void shutdown();

        int shutdownProc(boost::process::child& proc, int timeout);
    };




}