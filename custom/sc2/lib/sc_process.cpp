//
// Created by dewe on 7/16/22.
//

#include "util.h"
#include "stopwatch.h"
#include "sc_process.h"
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXUserAgent.h>

namespace sc2{
    StarcraftProcess::StarcraftProcess(const sc2::RunConfig &run_config, const std::filesystem::path &exec_path,
                                       const std::optional<sc2::Version> &version, bool full_screen,
                                       std::vector<std::string> const& extra_args, bool verbose,
                                       std::optional<std::string> const& host,
                                       std::optional<unsigned short> const& port, bool connect, std::optional<int> timeout,
                                       sc2::Point window_size, sc2::Point window_loc,
                                       std::vector<unsigned short> const& extra_ports) {

        checkExist(exec_path);
        m_tmpDir = std::tmpnam(nullptr);

        if(auto _tmp = run_config.tmpDir())
            m_tmpDir->replace_filename(*_tmp);
        else{
            auto _filename = m_tmpDir->stem().string();
            m_tmpDir->replace_filename(LOG_STREAM("sc-" << _filename));
        }

        m_host = host.value_or("127.0.0.1");

        if(port){
            m_port = port.value();
        }else if( auto s_port = get_env("SC2_PORT"); not s_port.empty()){
            m_port = std::stoi(s_port);
        }else{
            m_port = pickUnusedPort();
        }

        m_version = version;
        std::stringstream args;
        args << exec_path;
        addArg(args, "listen", m_host);
        addArg(args, "port", std::to_string(*m_port));
        addArg(args, "dataDir", run_config.dataDir());
        addArg(args, "tmpDir", *m_tmpDir);

        if(m_host.find(':') != std::string::npos){
            addArg(args, "ipv6");
        }
        if(verbose or getenv("SC2_VERBOSE")){
            addArg(args, "verbose");
        }
        if(getenv("SC2_VERBOSE_MP")){
            addArg(args, "verboseMP");
        }
        if(m_version and m_version->data_version){
            addArg(args, "dataVersion", upper(*m_version->data_version));
        }

        for(auto const& arg: extra_args)
            addArg(args, arg);

        if(std::getenv("SC2_GDB")){
            std::cerr << "Launching: gdb" << exec_path << "\n";
            std::cerr << "GDB run command:\n";
            std::cerr << "run " << args.str() << "\n\n";
            args = std::stringstream(LOG_STREAM("gdb" << args.str()));
        }else if(std::getenv("SC2_TRACE")){
            auto strace_out = "/tmp/sc2-strace.txt";
            std::cerr << "Launching in strace. Redirecting output to " << strace_out << "\n";
            args = std::stringstream(LOG_STREAM("strace -f -o " << strace_out << " " << args.str()));
        }else{
            std::cerr << "Launching SC2: " << args.str() << "\n";
        }

        With w{sw("startup")};
        if(not getenv("SC2_PORT")){
            this->m_proc = launch(run_config, args);
        }
        if (connect){
            this->m_controller = std::make_unique<RemoteController>(m_host, *m_port, this->m_proc.get());
        }
    }

    void StarcraftProcess::checkExist(std::filesystem::path const& path){

        if( not std::filesystem::is_regular_file(path)){
            throw std::runtime_error(LOG_STREAM("Trying to run " <<  path <<", but it doesn't exist" ));
        }

        if( (fs::status(path).permissions() & fs::perms::owner_read) == fs::perms::none){
            throw std::runtime_error(LOG_STREAM("Trying to run " <<  path <<", but it isn't executable."));
        }
    }

    StarcraftProcess::~StarcraftProcess() {
        sw.decorate("close")([this](){
            if(m_controller){
                m_controller->quit();
                m_controller.reset(nullptr);
            }

            shutdown();

            if(m_port){
                if(auto _port = get_env("SC2_PORT"); _port.empty()){
                    returnPort(*m_port);
                }
                m_port = {};
            }

            if(m_tmpDir and fs::exists(*m_tmpDir)){
                fs::remove(*m_tmpDir);
            }
        })();
    }

    std::unique_ptr<boost::process::child> StarcraftProcess::launch(const RunConfig &runConfig, const std::stringstream &args) {
        try{
            With w{sw("popen")};
            return std::make_unique<boost::process::child>(args.str());
        } catch (std::exception const& exp) {
            throw SC2LaunchError(LOG_STREAM( exp.what() << "\nFailed to launch: " << args.str()));
        }
    }

    void StarcraftProcess::shutdown() {
        if(m_proc){
            auto ret = shutdownProc(*m_proc, 3);
            std::cerr << "Shutdown with return code: " << ret << "\n";
            m_proc.reset(nullptr);
        }
    }

    int StarcraftProcess::shutdownProc(boost::process::child &proc, int timeout) {
        auto freq = 10;
        for(int i = 0; i < int(1+timeout * freq); i++){
            proc.terminate();
            auto terminated = not proc.running();
            if(terminated){
                std::cerr << "Shutdown gracefully.\n";
                return proc.exit_code();
            }
            std::this_thread::sleep_for(std::chrono::seconds(1/freq));
        }
        std::cerr << "Killing the Process.\n";
        proc.detach();
        proc.wait();
        return proc.exit_code();
    }

}
