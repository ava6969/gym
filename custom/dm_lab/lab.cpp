//
// Created by dewe on 10/30/21.
//

#include "opencv2/opencv.hpp"
#include "lab.h"

namespace gym{

    DMObservation Lab::observation(){
        DMObservation res;
        for(auto const&[name, data] : m_Lab.observations()){
            res[name] = data;
        }
        return res;
    }

    TimeStep<DMObservation> Lab::step(const std::map<std::string, int> &action) {
        if(needsReset){
            return reset();
        }

        for(auto[name, value] : action){
            m_labAction[m_ActionMap[name]] = value;
        }

        auto reward = static_cast<float>(m_Lab.step(m_labAction));

        if(m_Lab.isRunning())
            return transition(reward, observation());
        else{
            needsReset = true;
            return termination(reward, observation());
        }
    }

    TimeStep<DMObservation> Lab::reset() {
        m_Lab.reset();
        needsReset = false;
        return restart(observation()); // turn to visitor
    }

    Lab::Lab(const std::string &level_name, std::vector<std::string> observation_names,
             const std::map<std::string, std::string> &config, const std::string &renderer,
             LevelFetch* level_cache ):
            m_Lab(level_name,
                  observation_names,
                  config,
                  renderer,
                  level_cache,
                  {}),
            m_ObservationNames(std::move(observation_names)),
            needsReset(true){

        auto lab_action_specs = m_Lab.actionSpec();
        m_actionCount = (int)lab_action_specs.size();

        for(auto i = 0UL; i < lab_action_specs.size(); i++){
            auto[name, min, max] = lab_action_specs[i];
            m_ActionMap[name] = i;
            m_ActionSpec.emplace(name, BoundedArray<int>{{{}, DType::Int, name}, min, max});
        }

        for(auto const& [name, shape, type] : m_Lab.observationSpec()){
            m_ObservationSpec.emplace(name, Array{shape,
                                                  (type == EnvCApi_ObservationString ? DType::String :
                                                   type == EnvCApi_ObservationDoubles ? DType::Float : DType::Byte),
                                                  name});
        }

        m_labAction.resize(m_actionCount);
    }
}