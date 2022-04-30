//
// Created by dewe on 9/1/21.
//

#ifndef GYMENV_CLIP_REWARD_H
#define GYMENV_CLIP_REWARD_H

#include "common/wrapper.h"
//#include "atari/atari_env.h"

namespace gym {
    template<class EnvT>
    struct ClipRewardEnv : public RewardWrapper< EnvT > {

        explicit
        ClipRewardEnv( std::unique_ptr<EnvT> env ):RewardWrapper<EnvT>(std::move(env)) {}

        explicit ClipRewardEnv(std::shared_ptr<Space> obsSpace,
                               std::shared_ptr<Space> actSpace ):
                               RewardWrapper<EnvT>( obsSpace, actSpace ) {}

        inline float reward(float const& rew)  const noexcept final{
            return rew == 0 ? rew : rew > 0.f ? 1 : -1 ;
        }
    };
}

#endif //GYMENV_CLIP_REWARD_H
