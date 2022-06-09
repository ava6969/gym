//
// Created by dewe on 9/1/21.
//

#ifndef GYMENV_BASE_LAB_H
#define GYMENV_BASE_LAB_H

#include <functional>
#include <cstring>
#include <variant>
#include "dm_loader.h"
#include "memory"
#include "vector"
#include "filesystem"
#include "map"
#include "optional"

#define ENV_STATUS_CLOSED -3
#define ENV_STATUS_UNINITIALIZED -2
#define ENV_STATUS_INITIALIZED -1

using LabModuleState = char[4096];

static void get_module_state(LabModuleState& );

namespace cv{
    class Mat;
}

namespace gym{


    struct LevelFetch{
        virtual bool fetch(std::string const& key, std::string const& path) = 0;
        virtual void write(std::string const& key, std::string const& path) = 0;
        virtual ~LevelFetch()=default;
    };

    template<class T>
    struct Spec{
        std::string name{};
        T shape{};
        EnvCApi_ObservationType_enum type{};
    };

    using ObservationSpec = Spec<std::vector<int>>;
    struct ActionSpec{
        std::string name{};
        int min, max;
    };

    using DMObservation = std::unordered_map<std::string, cv::Mat>;

    class LabObject {

    public:

        LabObject(std::string const &level_name,
                  std::vector<std::string> const &observation_name,
                  const std::map<std::string, std::string>& config,
                  const std::string& renderer,
                  LevelFetch* levelCache=nullptr,
                  std::filesystem::path const& tempFolder={});

        bool reset(int _episode = -1, std::optional<int> const& _seed = std::nullopt);

        [[nodiscard]] inline
        bool isRunning() const {
            return (m_Status == ENV_STATUS_INITIALIZED) ||
                    (m_Status == EnvCApi_EnvironmentStatus_Running);
        }

        std::vector<ObservationSpec> observationSpec();

        inline static auto makeShape(EnvCApi_ObservationSpec_s const &spec){
            std::vector<int> dims(spec.dims);
            memcpy(dims.data(), spec.shape, sizeof(int) * spec.dims);
            return dims;
        }

        std::vector<ActionSpec>  actionSpec();

        static cv::Mat  makeObservation(const EnvCApi_Observation& observation);

        DMObservation observations();

        double step(std::vector<int> const& action, int numSteps=1);

        bool envClose();

        ~LabObject();

    private:
        EnvCApi m_Api{};
        void* m_Context{};
        int m_Status{};
        int m_Episode{};
        uint32_t m_ObservationCount{};
        std::vector<int> m_ObservationIndices{};
        int m_NumSteps{};
        LevelFetch* levelFetch;

        std::function<std::string(const std::string &)> log = [this](const std::string &log_message) {
            return log_message + "\"" + m_Api.error_message(m_Context) + "\"";
        };

        DeepMindLabLaunchParams makeParam(const std::string& renderer,
                                          LevelFetch* levelCache,
                                          std::filesystem::path const& tempFolder);

        void init(std::string const& levelName,
                  const std::map<std::string, std::string>& config,
                  const DeepMindLabLaunchParams& params);

        void createObservationSpace();

    };
}


#endif //GYMENV_BASE_LAB_H
