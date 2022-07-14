#pragma once
//
// Created by dewe on 7/14/22.
//

#include "vector"
#include "map"
#include "string"


namespace sc2 {

    enum class StepType{
        First=0, Mid=1, Last=2
    };

    template<class ObsT>
    struct TimeStep{
        StepType step_type;
        float reward{};
        float discount{};
        ObsT observation{};
    };

    static inline bool first(StepType s) noexcept { return s == StepType::First; }
    static inline bool mid(StepType s) noexcept { return s == StepType::Mid; }
    static inline bool last(StepType s) noexcept { return s == StepType::Last; }

    using ObsSpec = std::map< std::string, const std::vector<int> >;
    using ActionSpec = const std::vector<int>;

    template<class ObsT, class ActionT>
    class Base {

    public:
        virtual TimeStep<ObsT> reset() = 0;
        virtual TimeStep<ObsT> step(ActionT const& ) = 0;
        virtual std::vector< ObsSpec > observation_spec() = 0;
        virtual std::vector< ActionSpec > action_spec() = 0;
    };

} // sc2