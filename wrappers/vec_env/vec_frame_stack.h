//
// Created by dewe on 12/19/21.
//

#ifndef GYM_VEC_FRAME_STACK_H
#define GYM_VEC_FRAME_STACK_H

#include "vec_env_wrapper.h"
#include "common/stacked_observation.h"

namespace gym{

    using StringMap = std::unordered_map<std::string, std::string>;

    template<class VenvType, bool dict>
    class VecFrameStack : public VecEnvWrapper<VenvType, dict>{

//        template<class T=torch::Tensor>
//        using infer = std::conditional_t<dict, std::unordered_map<std::string, T>, T>;

        StackedObservation<dict> stackedObservation;
        int nStack;
    public:
        VecFrameStack( std::unique_ptr<VenvType> venv,
                       int nStack,
                       std::optional<std::string> const& channelOrder=std::nullopt):
                       VecEnvWrapper<VenvType, dict>( std::move(venv)),
                               stackedObservation( this->venv->nEnvs(), nStack,
                                                   this->venv->observationSpace(), channelOrder), nStack(nStack){
            this->m_ObservationSpace = stackedObservation.stackObservationSpace( this->venv->observationSpace() );
        }

        infer_type<dict> reset() noexcept override {
            return stackedObservation.reset(this->venv->reset());
        }

        typename VecEnvWrapper<VenvType, dict>::StepT stepWait() noexcept override{
            auto stepData = this->venv->stepWait();
            stepData.observation = stackedObservation.update(stepData.observation);
            return stepData;
        }

        infer_type<dict> reset(int index) noexcept override {
            return stackedObservation.update(this->venv->reset(index));
        }

        typename VecEnvWrapper<VenvType, dict>::StepT
        step(int index, torch::Tensor const& action) noexcept override{
            auto stepData = this->venv->step(index, action);
            stepData.observation = stackedObservation.update(stepData.observation);
            return stepData;
        }

        static auto make(std::unique_ptr<VenvType> x, int nStack,
                         std::optional<std::string>  const& channelOrder=std::nullopt){
            return std::make_unique<VecFrameStack<VenvType, dict>>( std::move(x), nStack, channelOrder);
        }

    };
}


#endif //GYM_VEC_FRAME_STACK_H
