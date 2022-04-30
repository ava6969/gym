//
// Created by dewe on 12/19/21.
//

#ifndef GYM_VEC_FRAME_STACK_H
#define GYM_VEC_FRAME_STACK_H

#include "vec_env_wrapper.h"
#include "common/stacked_observation.h"

namespace gym{

    using StringMap = std::unordered_map<std::string, std::string>;

    template<bool dict>
    class VecFrameStack : public VecEnvWrapper<dict>{

        StackedObservation<dict> stackedObservation;
        int nStack;
    public:
        VecFrameStack( std::unique_ptr< VecEnv<dict> > venv,
                       int nStack,
                       std::optional<std::string> const& channelOrder=std::nullopt);

        infer_type<dict> reset() noexcept override;

        typename VecEnvWrapper<dict>::StepT stepWait() noexcept override;

        infer_type<dict> reset(int index) noexcept override;

        typename VecEnvWrapper<dict>::StepT
        step(int index, torch::Tensor const& action) noexcept override;

        static std::unique_ptr<VecEnv<dict>>
        make(std::unique_ptr<VecEnv<dict>> x, int nStack=4,
             std::optional<std::string>  const& channelOrder=std::nullopt);

    };

    extern template class VecFrameStack<true>;
    extern template class VecFrameStack<false>;
}


#endif //GYM_VEC_FRAME_STACK_H
