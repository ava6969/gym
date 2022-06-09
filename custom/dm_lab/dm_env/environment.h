//
// Created by dewe on 9/1/21.
//

#ifndef GYMENV_ENVIRONMENT_H
#define GYMENV_ENVIRONMENT_H

#include <map>
#include "optional"
#include "specs.h"

namespace gym {
    enum class StepType {
        FIRST = 0,
        MID,
        LAST
    };

    template<typename ObservationType>
    struct TimeStep {
        StepType stepType{StepType::FIRST};
        std::optional<float> reward{};
        std::optional<float> discount{};
        std::unordered_map<std::string, cv::Mat> observation{};
    };

    static inline bool first(StepType type) { return type == StepType::FIRST; }

    static inline bool mid(StepType type) { return type == StepType::MID; }

    static inline bool last(StepType type) { return type == StepType::LAST; }

    template<typename ObservationType,
            typename ActionType>
    struct Environment {

        virtual TimeStep<ObservationType> reset() = 0;

        virtual TimeStep<ObservationType> step(std::map<std::string, ActionType> const &action) = 0;

        inline
        auto rewardSpec() const {
            return Array{{}, DType::Float, "reward"};
        }

        auto discountSpec() const {
            return BoundedArray<float>{{{}, DType::Float, "discount"}, 0.f, 1.f};
        }

        auto restart(ObservationType const &observation) {
            return TimeStep<ObservationType>{StepType::FIRST, std::nullopt, std::nullopt, observation};
        }

        auto transition(float reward, ObservationType const &observation, float discount = 1.0f) {
            return TimeStep<ObservationType>{StepType::MID, reward, discount, observation};
        }

        auto termination(float reward, ObservationType const &observation) {
            return TimeStep<ObservationType>{StepType::LAST, reward, 0.0f, observation};
        }

        auto truncation(float reward, ObservationType const &observation, float discount = 1.0f) {
            return TimeStep<ObservationType>{StepType::LAST, reward, discount, observation};
        }
    };
}

#endif //GYMENV_ENVIRONMENT_H
