//
// Created by dewe on 9/1/21.
//

#ifndef GYMENV_FIRE_RESET_H
#define GYMENV_FIRE_RESET_H

#include "common/wrapper.h"
//#include "atari/atari_env.h"

namespace gym {
    struct FireResetEnv : public Wrapper< Env< ObsT<true>, int> > {

    public:
        DEFAULT_RESET_OVERRIDE
        DEFAULT_STEP_OVERRIDE
        DEFAULT_STEP_OVERRIDES

        explicit FireResetEnv(std::unique_ptr<gym::Env<ObsT<true>, int>> env):
        Wrapper(std::move(env)){ }

        ObsT<true> reset()  noexcept  override;

    };
}
#endif //GYMENV_FIRE_RESET_H
