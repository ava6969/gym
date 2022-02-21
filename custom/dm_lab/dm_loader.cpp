//
// Created by dewe on 6/8/21.
//

#include "dm_loader.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __APPLE__
#  include <copyfile.h>
#else  // defined(__APPLE__)
#  include <sys/sendfile.h>
#endif  // defined(__APPLE__)

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>

#ifdef LEAK_SANITIZER
#include <sanitizer/lsan_interface.h>
#endif

#include "dm_loader.h"

using namespace std::string_literals;

namespace gym {


    void DMLoader::close_handle(EnvCApi & envCApi, void* context) {
        std::lock_guard<std::mutex> connect_guard(m_ConnectMutex);
        auto it = m_InternalContext.find(context);

        if( it  != m_InternalContext.end()) {
            envCApi.release_context(context);
#ifdef LEAK_SANITIZER
            // This function is usually called by LSAN at the end of the process.
    // Since dlclose is somewhat like an end of process as far as the DSO
    // is concerned, we call LSAN eagerly here. This prevents LSAN from
    // considering still-reachable DSO-global allocations as overall leaks.
    // This call effectively ends the use of LSAN in this process, since
    // future calls of this function are no-ops. We will therefore only
    // detect leaks that have happened up until now, but in typical uses,
    // there will be only one single dlclose near the end of the program.
    //
    // We have tried hard to minimize the amount of such leaks. It is worth
    // checking periodically (by disabling the following line) how much each
    // DSO load leaks, though, to make sure no large regressions sneak back
    // in. The only culprits at the moment are various OpenGL libraries.
    //
    // Note that it can be tricky to symbolize LSAN backtraces after the DSO
    // has been unloaded. You will at least want to make a note of the process
    // module maps just before the return from dmlab_connect below, e.g. via:
    // std::cerr << "Maps:\n" << std::ifstream("/proc/self/maps").rdbuf();

    __lsan_do_leak_check();
#endif
            dlclose(it->second);
            m_InternalContext.erase(it);
        }
    }

    ssize_t copy_complete_file(int in_fd, int out_fd) {
#ifdef __APPLE__
        return fcopyfile(in_fd, out_fd, nullptr, COPYFILE_ALL);
#else  // defined(__APPLE__)
        off_t offset = 0;
        struct stat stat_in;

        if (fstat(in_fd, &stat_in) == -1) {
            std::cerr << "Failed to read source filesize\n";
            return -1;
        }

        for (ssize_t count = stat_in.st_size, bytes_read = 0; bytes_read < count;) {
            ssize_t res = sendfile(out_fd, in_fd, &offset, count - bytes_read);
            if (res < 0) {
                // An error occurred.
                if (errno == EINTR || errno == EAGAIN) {
                    // e.g. intervening interrupt, just keep trying
                    continue;
                } else {
                    // unrecoverable error
                    return res;
                }
            } else if (res == 0) {
                // The file was shorter than fstat originally reported, but that's OK.
                return 0;
            } else {
                // No error, res bytes were read.
                bytes_read += res;
            }
        }

        return 0;
#endif  // defined(__APPLE__)
    }

    int DMLoader::connect(const gym::DeepMindLabLaunchParams &params,
                          EnvCApi &env_c_api,
                          void **context) {

        std::lock_guard<std::mutex> connect_guard(m_ConnectMutex);

        auto so_path = params.runFilesPath;

        if (not params.runFilesPath.empty()) {
            switch (params.renderer) {
                case DeepMindLabRenderer::DeepMindLabRenderer_Software:
                    so_path /= "libdmlab_headless_sw.so";
                    break;
                case DeepMindLabRenderer::DeepMindLabRenderer_Hardware:
                    so_path /= "libdmlab_headless_hw.so";
                    break;
            }
        } else {
            std::cerr << "Require runfiles_directory!\n";
            return 1;
        }

        if(void* dlhandle = openUniqueDSO(so_path)){

            for (const auto& [_, ctx_dlhandle] : m_InternalContext) {
                if (ctx_dlhandle == dlhandle) {
                    std::cerr << "Failed to create new instance of library!\n";
                    return 1;
                }
            }
            using EnvCApiConnect = int(const DeepMindLabLaunchParams* params,
                                       EnvCApi* env_c_api, void** context);
            EnvCApiConnect* connect;
            *reinterpret_cast<void**>(&connect) = dlsym(dlhandle, "dmlab_connect");
            if (connect == nullptr) {
                std::cerr << "Failed to find function dmlab_connect in library!\n";
                return 1;
            }

            int connect_status = connect(&params, &env_c_api, context);
            if (connect_status != 0) {
                std::cerr << "Failed to call function dmlab_connect, return value was: "
                          << connect_status << "\n";
                return 1;
            }

            m_InternalContext.emplace(*context, dlhandle);
            return 0;
        }
        return 1;
    }

    DMLoader* DMLoader::m_Instance = nullptr;

    void *DMLoader::openUniqueDSO(const std::filesystem::path &so_path) {

        int current = m_GlobalCounter++;
        void* dlhandle;

        if (current == 0) {
            dlhandle = dlopen(so_path.c_str(), RTLD_NOW | RTLD_LOCAL);
        } else {
            int source_fd = open(so_path.c_str(), O_RDONLY, 0);
            if (source_fd < 0) {
                std::cerr << "Failed to open library: \"" << so_path << "\"\n"
                          << errno << " - " << std::strerror(errno) << "\n";
                return nullptr;
            }

            std::stringstream ss;
            auto suffix = "_dmlab.so"s;
            ss << "unique_dso_filename_" << current << "_XXXXXX" << suffix;
            auto temp_path_ = std::filesystem::temp_directory_path() / ss.str();
            std::string temp_path = temp_path_.string();

            int dest_fd = mkstemps(&temp_path.front(), (int)suffix.size());

            if (dest_fd < 0) {
                std::cerr << "Failed to make library: \"" << temp_path << "\"\n"
                          << errno << " - " << std::strerror(errno) << "\n";
                close(source_fd);
                return nullptr;
            }

            if (copy_complete_file(source_fd, dest_fd) < 0) {
                std::cerr << "Failed to copy file to destination \"" << temp_path
                          << "\"\n" << errno << " - " << std::strerror(errno) << "\n";
                std::remove(temp_path.c_str());
                close(dest_fd);
                close(source_fd);
                return nullptr;
            }

            dlhandle = dlopen(temp_path.c_str(), RTLD_NOW | RTLD_LOCAL);
            std::remove(temp_path.c_str());
            close(dest_fd);
            close(source_fd);
        }

        if (dlhandle == nullptr) {
            std::cerr << "Failed to open library! - " << so_path << "\n"
                      << dlerror() << "\n";
        }

        return dlhandle;
    }
}