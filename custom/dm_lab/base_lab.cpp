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

    bool LabObject::envClose() {
        if(m_Status != ENV_STATUS_CLOSED){
            DMLoader::instance()->close_handle(m_Api, m_Context);
            m_Status = ENV_STATUS_CLOSED;
            return true;
        }
        if(levelFetch){
            delete levelFetch;
            levelFetch = nullptr;
        }
        return false;
    }

    LabObject::LabObject(const std::string &level_name,
                         const std::vector<std::string> &observations,
                         const std::map<std::string, std::string>& config,
                         const std::string& renderer,
                         LevelFetch* levelCache,
                         const std::filesystem::path &tempFolder):
                         m_ObservationCount(observations.size()),
                         m_ObservationIndices(m_ObservationCount),
                         m_Status(ENV_STATUS_CLOSED){

        auto param = makeParam(renderer, levelCache, tempFolder);

        init(level_name, config, param);

        createObservationSpace();
        levelFetch = levelCache;
    }

    void LabObject::init(std::string const& levelName,
                         const std::map<std::string, std::string>& config,
                         const DeepMindLabLaunchParams& params) {

        if (DMLoader::instance()->connect(params, m_Api, &m_Context) != 0) {
            throw std::runtime_error("Failed to connect to RL API");
        }

#ifndef __has_feature
#  define __has_feature(x) 0
#endif
#if __has_feature(thread_sanitizer)
        if (m_Api.setting(m_Context, "vmMode", "interpreted") != 0) {
            throw std::runtime_error( "Failed to apply 'vmMode' setting - \"%s\"");
        }
#endif

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
                                                 LevelFetch* levelCache,
                                                 std::filesystem::path const& tempFolder) {
        DeepMindLabLaunchParams params = {};
        auto home = std::filesystem::path( std::getenv("HOME") );
        params.runFilesPath = home / "sam" / "gym" / "custom" / "dm_lab" /  "lab" / "org_deepmind_lab";

        if (renderer == "hardware") {
            params.renderer = DeepMindLabRenderer::DeepMindLabRenderer_Hardware;
        } else if (renderer != "software") {
            throw std::runtime_error("Failed to set renderer must be \"hardware\" or "
                                     "\"software\" actual \"" + renderer + "\"!");
        }

        if(levelCache){
            params.levelCacheParams.context = levelCache;

            params.levelCacheParams.fetch_level_from_cache = [](
                    void * level_cache_context,
                    const char *const cache_paths[],
                    int num_cache_paths,
                    const char * key,
                    const char * pk3_path) -> bool {

                if(level_cache_context){
                    return reinterpret_cast<LevelFetch*>(level_cache_context)->fetch(key, pk3_path);
                }else{
                    throw std::runtime_error("LevelCache* has been deallocated before call in dmlab engine");
                }
            };

            params.levelCacheParams.write_level_to_cache = [](void * level_cache_context, const char *const *, int,
                                                              const char * key , const char * pk3_path)  -> void {
                if(level_cache_context){
                    reinterpret_cast<LevelFetch*>(level_cache_context)->fetch(key, pk3_path);
                }else{
                    throw std::runtime_error("LevelCache* has been deallocated before call in dmlab engine");
                }
            };

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
            throw std::runtime_error( log("Environment Error") );
        }

        m_NumSteps = 0;
        ++m_Episode;
        m_Status = ENV_STATUS_INITIALIZED;
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
            auto msg = log("Failed to advance Environment");
            DMLoader::instance()->close_handle(m_Api, m_Context);
            throw std::runtime_error(msg);
        }

        return reward;
    }

    std::vector<ObservationSpec> LabObject::observationSpec() {

        EnvCApi_ObservationSpec_s spec{};
        auto count = m_Api.observation_count(m_Context);
        std::vector<ObservationSpec> result(count);

        for(int i = 0; i < count; i++){
            m_Api.observation_spec(m_Context, i, &spec);
            auto shape = spec.type != EnvCApi_ObservationString ? std::vector<int>{} : makeShape(spec);
            result[i] = { m_Api.observation_name(m_Context, i), shape, spec.type};
        }
        return result;
    }

    std::vector<ActionSpec> LabObject::actionSpec() {
        auto count = m_Api.action_discrete_count(m_Context);
         std::vector<ActionSpec> result(count);

        int min_discrete, max_discrete;
        for (int i = 0; i < count; ++i) {
            m_Api.action_discrete_bounds(m_Context, i, &min_discrete, &max_discrete);
            result[i] = {m_Api.action_discrete_name(m_Context, i), min_discrete, max_discrete};
        }

        return result;
    }

    cv::Mat LabObject::makeObservation(const EnvCApi_Observation &observation) {
        auto spec = observation.spec;
        switch (observation.spec.type) {

            case EnvCApi_ObservationString:
                return cv::Mat1b(1, int(spec.shape[0]), reinterpret_cast<uchar*>(
                        const_cast<char*>(observation.payload.string )));
            case EnvCApi_ObservationDoubles:
            {
                std::vector<double> array(spec.dims);
                memcpy(array.data(), observation.payload.doubles, spec.dims * sizeof(double ));
                return cv::Mat1d(1, spec.dims, const_cast<double*>(observation.payload.doubles));
            }
            default:{

                auto[row, col, channel] = std::make_tuple(spec.shape[0], spec.shape[1], spec.shape[2]);
                auto data = const_cast<uchar*>(observation.payload.bytes);
                return {row, col, CV_8UC(channel), data};
            }
        }
    }

    DMObservation LabObject::observations() {
        if(not isRunning()){
            throw std::runtime_error( "Environment in wrong status for call to observations()");
        }

        DMObservation result;
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