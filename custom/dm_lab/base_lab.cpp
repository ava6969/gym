//
// Created by dewe on 9/1/21.
//

#include "base_lab.h"
#include "random"
#include <opencv2/opencv.hpp>

namespace gym{

    LabObject::~LabObject() {
        envClose();
    }

    int LabObject::envClose() {
        if(m_Status != ENV_STATUS_CLOSED){
            DMLoader::instance()->close_handle(m_Api, m_Context);
            m_Status = ENV_STATUS_CLOSED;
            return 1;
        }
        return 0;
    }



    LabObject::LabObject(const std::string &level_name,
                         const std::vector<std::string> &observations,
                         const std::map<std::string, std::string>& config, const std::string& renderer,
                         void* levelCache,
                         const std::filesystem::path &tempFolder):
                         m_ObservationCount(observations.size()),
                         m_ObservationIndices(m_ObservationCount){

        auto param = makeParam(renderer, levelCache, tempFolder);

        init(level_name, config, param);

        createObservationSpace();
    }

    void LabObject::init(std::string const& levelName,
                         const std::map<std::string, std::string>& config,
                         const DeepMindLabLaunchParams& params) {

        auto log = [&](const std::string &log_message) {
            return log_message + "\"" + m_Api.error_message(m_Context) + "\"";
        };

        if (DMLoader::instance()->connect(params, m_Api, &m_Context) != 0) {
            throw std::runtime_error("Failed to connect to RL API");
        }

        m_Status = ENV_STATUS_UNINITIALIZED;
        m_Episode = 0;

        if (m_Api.setting(m_Context, "levelName", levelName.c_str()) != 0) {
            throw std::runtime_error(log("Invalid levelName Flag " + levelName));
        }

        if (m_Api.setting(m_Context, "fps", "60") != 0) {
            throw std::runtime_error(log("Failed to set fps "));
        }

        for(auto const& [key, value] : config){
            if(m_Api.setting(m_Context, key.c_str(), value.c_str()) != 0){
                throw std::runtime_error(log(std::string("Failed to apply setting ")
                                                     .append(key)
                                                     .append(" = ")
                                                     .append(value)
                                                     .append(" - ")));
            }
        }

        if(m_Api.init(m_Context) != 0){
            throw std::runtime_error(log("Failed to init environment"));
        }
    }

    DeepMindLabLaunchParams LabObject::makeParam(const std::string& renderer,
                                                 void* levelCache,
                                                 std::filesystem::path const& tempFolder) {
        DeepMindLabLaunchParams params = {};
        params.runFilesPath= getenv("DM_RUNFILES_PATH");
        if(params.runFilesPath.empty())
            params.runFilesPath = std::filesystem::current_path() / "deepmind_lab.so.runfiles" / "org_deepmind_lab";

        if (renderer == "hardware") {
            params.renderer = DeepMindLabRenderer::DeepMindLabRenderer_Hardware;
        } else if (renderer != "software") {
            throw std::runtime_error("Failed to set renderer must be \"hardware\" or "
                                     "\"software\" actual \"" + renderer + "\"!");
        }

        if(levelCache){
            params.levelCacheParams.context = levelCache;
            params.levelCacheParams.fetch_level_from_cache = [](void *, const char *const *, int,
                                                                const char *, const char *) -> bool {
                return false;
            };

            params.levelCacheParams.write_level_to_cache = [](void *, const char *const *, int,
                                                              const char *, const char *)  -> void {

            };
            m_LevelCache = levelCache;
        }

        params.optionalTempFolder = tempFolder;
        return params;
    }

    void LabObject::createObservationSpace() {

        std::string observationName{};
        int api_observation_count = m_Api.observation_count(m_Context);

        for (int i = 0; i < m_ObservationCount; ++i) {
            observationName = m_Api.observation_name(m_Context, i);
            if(observationName.empty()){
                throw std::runtime_error("dmlab observation was empty");
            }

            int j;
            for (j = 0; j < api_observation_count; ++j) {
                if (observationName == m_Api.observation_name(m_Context, j)) {
                    m_ObservationIndices[i] = j;
                    break;
                }
            }

            if(j == api_observation_count)
            {
                throw std::runtime_error(std::string("Unknown Observation - ").append(observationName));
            }
        }
    }

    bool LabObject::reset(int _episode,
                          std::optional<int> const& _seed) {

        int seed{};
        if (_episode >= 0)
            m_Episode = _episode;

        if (!_seed) {
            std::random_device rd;
            std::mt19937 g(rd());
            seed = std::uniform_int_distribution<int>()(g);
        } else {
            seed = *_seed;
        }

        if (m_Api.start(m_Context, m_Episode, seed) != 0) {
            throw std::runtime_error( m_Log("Environment Error") );
        }

        m_NumSteps = 0;
        ++m_Episode;
        m_Status = EnvCApi_EnvironmentStatus_Running;
        return true;
    }

    double LabObject::step(std::vector<int> const& action, int numSteps) {

        double reward;

        if (not isRunning())
            throw std::runtime_error("Environment in wrong status to advance");

        int action_discrete_count = m_Api.action_discrete_count(m_Context);

        if (action.size() != (size_t) action_discrete_count) {
            throw std::runtime_error("Action must have shape [ " + std::to_string(action_discrete_count) + " ] ");
        } else {
            m_Api.act_discrete(m_Context, action.data());
        }

        m_Status = m_Api.advance(m_Context, numSteps, &reward);
        m_NumSteps += numSteps;

        if (m_Status == EnvCApi_EnvironmentStatus_Error) {
            auto msg = m_Log("Failed to advance Environment");
            DMLoader::instance()->close_handle(m_Api, m_Context);
            throw std::runtime_error(msg);
        }

        return reward;
    }

    std::vector<std::tuple<std::string,
            std::vector<int>,
            EnvCApi_ObservationType_enum>> LabObject::observationSpec() {

        std::vector<std::tuple<std::string, std::vector<int>, EnvCApi_ObservationType_enum>> result{};
        EnvCApi_ObservationSpec_s spec{};
        int j = 0;

        for(auto idx : m_ObservationIndices){
            m_Api.observation_spec(m_Context, idx, &spec);
            result.push_back({m_Api.observation_name(m_Context, idx),
                              {},
                              spec.type});
            if (spec.type != EnvCApi_ObservationString) {
                std::vector<int> dims(spec.dims);
                memcpy(dims.data(), spec.shape, sizeof(int) * spec.dims);
                std::get<1>(result[j]) = dims;
            }
            j++;
        }
//
//        auto m_Count = m_Api.observation_count(m_Context);
//        std::vector<std::tuple<std::string, std::vector<int>, EnvCApi_ObservationType_enum>> result(m_Count);
//        EnvCApi_ObservationSpec_s spec{};

//        for(int i=0; i < m_Count; i++) {
//            m_Api.observation_spec(m_Context, i, &spec);
//
//            result[i] = {m_Api.observation_name(m_Context, i),
//                         {},
//                         spec.type};
//
//            if (spec.type != EnvCApi_ObservationString) {
//                std::vector<int> dims(spec.dims);
//                memcpy(dims.data(), spec.shape, sizeof(int) * spec.dims);
//                std::get<1>(result[i]) = dims;
//            }
//        }
        return result;
    }

    std::vector<std::tuple<std::string, int, int>> LabObject::actionSpec() {
        auto count = m_Api.action_discrete_count(m_Context);
         std::vector<std::tuple<std::string, int, int>> result(count);

        int min_discrete, max_discrete;
        for (int i = 0; i < count; ++i) {
            m_Api.action_discrete_bounds(m_Context, i, &min_discrete,
                                                    &max_discrete);
            result[i] = {m_Api.action_discrete_name(m_Context, i), min_discrete, max_discrete};
        }

        return result;
    }

    DMObservation LabObject::makeObservation(const EnvCApi_Observation &observation) {
        auto spec = observation.spec;
        switch (observation.spec.type) {

            case EnvCApi_ObservationString:
                return observation.payload.string;
            case EnvCApi_ObservationDoubles:
            {
                std::vector<double> array(spec.dims);
                memcpy(array.data(), observation.payload.doubles, spec.dims * sizeof(double ));
                return array;
            }
            default:{

                auto[row, col, channel] = std::make_tuple(spec.shape[0], spec.shape[1], spec.shape[2]);
                auto data = const_cast<uchar*>(observation.payload.bytes);

                cv::Mat m_Mat = cv::Mat(row, col, CV_8UC(channel), cv::Scalar::all(0));

                memcpy(m_Mat.data, data, spec.shape[0] * spec.shape[1] * spec.shape[2] * sizeof(uint8_t));

                return m_Mat;
            }
        }
    }

    std::unordered_map<std::string, DMObservation>  LabObject::observations() {
        if(not isRunning()){
            throw std::runtime_error( "Environment in wrong status for call to observations()");
        }

        std::unordered_map<std::string, DMObservation> result;

        EnvCApi_Observation observation;

        for (int i = 0; i < m_ObservationCount; ++i) {
            auto idx = m_ObservationIndices[i];
            m_Api.observation(m_Context, idx, &observation);
            result.emplace( m_Api.observation_name(m_Context, idx),
                            makeObservation(observation));
        }
        return result;
    }

}