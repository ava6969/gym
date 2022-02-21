//
// Created by dewe on 12/21/21.
//

#ifndef GYM_TIME_LIMIT_H
#define GYM_TIME_LIMIT_H

#include "common/wrapper.h"

namespace gym{

    template<class EnvT>
    class TimeLimit : public Wrapper<typename EnvT::ObservationT, typename EnvT::ActionT, typename EnvT::StepT> {

    public:

        static auto make(std::unique_ptr<EnvT> env,
                  std::optional<size_t> const& max_episode_steps){
            return std::make_unique<TimeLimit<EnvT>>( std::move(env), max_episode_steps);
        }

        TimeLimit(std::unique_ptr<EnvT> env,
                  std::optional<size_t> const& max_episode_steps):
        Wrapper<typename EnvT::ObservationT, typename EnvT::ActionT, typename EnvT::StepT>( std::move(env) ){
            if( max_episode_steps )
                maxEpisodeSteps = max_episode_steps.value();
        }

        inline typename EnvT::StepT step( typename EnvT::ActionT const& action) noexcept{
            auto&& response = this->m_Env->step( action );
            elapsedSteps++;
            if (elapsedSteps >= maxEpisodeSteps){
                response.info["TimeLimit.truncated"] = not response.done;
                response.done = true;
            }
            return response;
        }

        inline typename EnvT::ObservationT reset() noexcept{
            elapsedSteps = 0;
            return this->m_Env->reset();
        }

    private:
        size_t maxEpisodeSteps{ std::numeric_limits<size_t>::max() };
        int64_t elapsedSteps{};
    };
}
#endif //GYM_TIME_LIMIT_H
