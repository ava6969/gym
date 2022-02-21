//
// Created by dewe on 6/8/21.
//

#ifndef FASTDRL1_DM_LOADER_H
#define FASTDRL1_DM_LOADER_H


#include <stddef.h>
#include "mutex"
#include "lab/public/file_reader_types.h"
#include "lab/public/level_cache_types.h"
#include "lab/public/env_c_api.h"
#include "filesystem"
#include "unordered_map"

namespace gym
{
    enum class DeepMindLabRenderer {
        DeepMindLabRenderer_Software,
        DeepMindLabRenderer_Hardware,
    };

    struct DeepMindLabLaunchParams {
        std::filesystem::path runFilesPath;
        DeepMindLabRenderer renderer;
        DeepMindLabLevelCacheParams levelCacheParams;
        DeepmindFileReaderType* fileReaderOverride;
        std::filesystem::path optionalTempFolder;
        const DeepMindReadOnlyFileSystem* readOnlyFileSystem;
    };

    class DMLoader{

    public:
        int connect(DeepMindLabLaunchParams const & params,
                    EnvCApi& env_c_api,
                    void** context);

        static inline
        DMLoader* instance(){
            if(m_Instance)
                return m_Instance;

            m_Instance = new DMLoader();
            return m_Instance;
        }

        ~DMLoader() { delete m_Instance; }

        void close_handle(EnvCApi & envCApi, void* context);

    private:
        DMLoader()=default;

        std::mutex m_ConnectMutex;

        std::unordered_map<void*, void*> m_InternalContext{};

        static DMLoader* m_Instance;

        int m_GlobalCounter{};

        void* openUniqueDSO(const std::filesystem::path& so_path);

    };

}

#endif //FASTDRL1_DM_LOADER_H
