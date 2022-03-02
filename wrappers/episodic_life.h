//
// Created by dewe on 9/1/21.
//

#ifndef GYMENV_EPISODIC_LIFE_H
#define GYMENV_EPISODIC_LIFE_H

#include "common/wrapper.h"
//#include "atari/atari_env.h"

namespace gym {

    class EpisodicLifeEnv : public Wrapper< ObsT<true>, int> {

    private:
         int m_Lives{0};
         bool m_WasRealDone{true};
         AtariEnv<true>* parent = try_cast<AtariEnv<true>>();

    public:
        DEFAULT_RESET_OVERRIDE
        DEFAULT_STEP_OVERRIDE

        explicit EpisodicLifeEnv(std::unique_ptr<gym::Env<ObsT<true>, int>> env):Wrapper(std::move(env)){
            if(not parent){
                throw std::runtime_error("EpisodicLifeEnv only supports wrapping AtariEnv");
            }
        }

        StepResponse<ObsT<true>> step(const int &action) noexcept override;

        ObsT<true> reset() noexcept override;
    };


}

#endif //GYMENV_EPISODIC_LIFE_H
