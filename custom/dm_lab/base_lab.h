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

namespace cv{
    class Mat;
}

namespace gym{

    using DMObservation = std::variant<std::string,
            cv::Mat,
            std::vector<double>>;

    class LabObject {

    public:

        LabObject(std::string const &level_name,
                  std::vector<std::string> const &observation_name,
                  const std::map<std::string, std::string>& config,
                  const std::string& renderer,
                  void* levelCache={},
                  std::filesystem::path const& tempFolder={});

        bool reset(int _episode = -1, std::optional<int> const& _seed = std::nullopt);

        [[nodiscard]] inline
        bool isRunning() const {
            return m_Status == ENV_STATUS_INITIALIZED || m_Status == EnvCApi_EnvironmentStatus_Running;
        }

        std::vector<std::tuple<std::string,
        std::vector<int>,
        EnvCApi_ObservationType_enum>> observationSpec();

        std::vector<std::tuple<std::string, int, int>> actionSpec();

        std::variant<std::string,
        cv::Mat,
        std::vector<double>> makeObservation(const EnvCApi_Observation& observation);

        std::unordered_map<std::string, DMObservation> observations();

        double step(std::vector<int> const& action, int numSteps=1);

        int envClose();

        ~LabObject();

    private:
        EnvCApi m_Api{};
        void* m_Context{};
        void* m_LevelCache{};
        int m_Status{};
        int m_Episode{};
        int m_ObservationCount{};
        std::vector<int> m_ObservationIndices{};
        int m_NumSteps{};

        std::function<std::string(const std::string &)> m_Log = [&](const std::string &log_message) {
            return log_message + "\"" + m_Api.error_message(m_Context) + "\"";
        };

        DeepMindLabLaunchParams makeParam(const std::string& renderer,
                                          void* levelCache,
                                          std::filesystem::path const& tempFolder);

        void init(std::string const& levelName,
                  const std::map<std::string, std::string>& config,
                  const DeepMindLabLaunchParams& params);

        void createObservationSpace();

    };
}


#endif //GYMENV_BASE_LAB_H
