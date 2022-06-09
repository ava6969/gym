//
// Created by dewe on 6/18/21.
//

#pragma once

#include "string"
#include "custom/dm_lab/dm_env/environment.h"
#include <unsupported/Eigen/CXX11/Tensor>
#include <utility>
#include "base_lab.h"

namespace gym {

    class Lab : public Environment<DMObservation, int>{

    public:
        Lab(std::string const &level_name,
            std::vector<std::string> observation_names,
            const std::map<std::string, std::string>& config,
            std::string const& renderer,
            LevelFetch* level_cache= nullptr);

        TimeStep<DMObservation> reset() override;

        TimeStep<DMObservation> step(const std::map<std::string, int> &action) override ;

        inline
        std::unordered_map<std::string, BoundedArray<int>> actionSpec() const { return m_ActionSpec; }

        inline
        std::unordered_map<std::string, Array> observationSpec() const { return m_ObservationSpec; }

    private:
        LabObject m_Lab;
        std::vector<std::string> m_ObservationNames;
        bool needsReset{true};
        int m_actionCount;

        std::unordered_map<std::string, int> m_ActionMap;
        std::unordered_map<std::string, BoundedArray<int>> m_ActionSpec;
        std::unordered_map<std::string, Array> m_ObservationSpec;
        std::vector<int> m_labAction;

        DMObservation observation();
    };
}