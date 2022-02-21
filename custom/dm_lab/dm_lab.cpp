//
// Created by dewe on 10/9/21.
//

#include "opencv2/opencv.hpp"
#include "dm_lab.h"

namespace gym{

    DMLabEnv::DMLabEnv(OptionalArgMap const& args):
            TensorEnv<DMLabEnv>(args),
            m_numSteps(getArg("action_repeats", 1)),
            m_Lab(getArg("game", std::string("lt_chasm")),
                  getArg("observation_names", std::vector<std::string>{"RGB_INTERLEAVED"}),
                  [&]() -> std::map<std::string, std::string> {
                  auto isTest = getArg("is_test", false);
                  std::map<std::string, std::string> config;
                  if(isTest){
                      config["allowHoldOutLevels"] = "true";
                      config["mixerSeed"] = "0x600D5EED";
                  }
                  auto dp = getArg<std::string>("dataset_path", "");
                  if(not dp.empty())
                      config["datasetPath"] = dp;

                  return config;
              }(),
              getArg<std::string>("renderer", "hardware"),
                      getArg("level_cache_param", DeepMindLabLevelCacheParams_s{})),
            m_ActionSet(getArg<std::vector<std::map<string, int>>>("action_set", DEFAULT_ACTION_SET)){

        NamedSpaces spaces;
        for(auto const&[name, spec] : m_Lab.observationSpec()){
            auto int64Shape = std::vector<int64_t>(spec.shape.begin(), spec.shape.end());

            if(spec.dType == DType::Float){
                spaces[name] = makeBoxSpace<float>(int64Shape);
            }else if(spec.dType == DType::Byte){
                spaces[name] = makeBoxSpace<uint8_t>(0, 255, int64Shape);
            }else {
                throw std::runtime_error("dmlab observation isn't currently setup to work with int and string");
            }
        }

        m_ObservationSpace = makeDictionarySpace(move(spaces));
        m_ActionSpace = makeDiscreteSpace(m_ActionSet.size());

// todo: use action type as specialization for class
//            for(auto const&[name, spec] : m_Lab.actionSpec()){
//                if(spec.dType == DType::Int){
//                    if(spec.shape.empty()){
//                        spaces[name] = makeDiscreteSpace(actionSet.size());
//                    }else{
//                        throw std::runtime_error("dmlab discrete actions cant be multidiscrete");
//                    }
//                }else{
//                    throw std::runtime_error("dmlab actions is only set for discrete");
//                }
//            }
    }

    TensorDict DMLabEnv::reset() noexcept {
        return to_tensor_map(m_Lab.reset().observation);
    }

    StepResponse<TensorDict> DMLabEnv::step(const torch::Tensor &action) noexcept {
        auto observation = m_Lab.step(m_ActionSet[action.item<int>()]);
        std::unordered_map<std::string, DMObservation> m_LastObservation = observation.observation;
        return {to_tensor_map(m_LastObservation),
                observation.reward.value(),
                last(observation.stepType),
                {}};
    }

    void DMLabEnv::render(RenderType ) {
//        auto mat = std::get<cv::Mat>(m_LastObservation["RGB_INTERLEAVED"]);
    }
}