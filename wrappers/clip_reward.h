//
// Created by dewe on 9/1/21.
//

#ifndef GYMENV_CLIP_REWARD_H
#define GYMENV_CLIP_REWARD_H

#include "common/wrapper.h"
#include "atari/atari_env.h"

namespace gym {
    template<bool image>
    struct ClipRewardEnv : public RewardWrapper<ObsT<image>, int> {

        explicit
        ClipRewardEnv( std::unique_ptr<gym::Env<ObsT<image>, int>> env ):
        RewardWrapper<ObsT<image>, int>(std::move(env)) {}

        explicit ClipRewardEnv(std::shared_ptr<Space> obsSpace,
                               std::shared_ptr<Space> actSpace ):
                               RewardWrapper<ObsT<image>, int>( obsSpace, actSpace ) {}

        inline float reward(float const& rew)  const noexcept final{
            return rew == 0 ? rew : rew > 0.f ? 1 : -1 ;
        }
    };
}

#endif //GYMENV_CLIP_REWARD_H
