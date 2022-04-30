//
// Created by dewe on 12/19/21.
//

#include "vec_frame_stack.h"

namespace gym {

    template<bool dict>
    VecFrameStack<dict>::VecFrameStack(std::unique_ptr<VecEnv<dict> > venv,
                                       int nStack,
                                       std::optional<std::string> const &channelOrder):
            VecEnvWrapper<dict>(std::move(venv)),
            stackedObservation(this->venv->nEnvs(),
                               nStack,
                               this->venv->observationSpace(), channelOrder), nStack(nStack) {

        this->m_ObservationSpace = stackedObservation.stackObservationSpace( this->venv->observationSpace().get() );

    }

    template<bool dict>
    infer_type<dict> VecFrameStack<dict>::reset() noexcept {
        return stackedObservation.reset(this->venv->reset());
    }

    template<bool dict>
    typename VecEnvWrapper<dict>::StepT VecFrameStack<dict>::stepWait() noexcept {
        auto stepData = this->venv->stepWait();
        stepData.observation = stackedObservation.update(stepData.observation);
        return stepData;
    }

    template<bool dict>
    infer_type<dict> VecFrameStack<dict>::reset(int index) noexcept {
        return stackedObservation.update(this->venv->reset(index));
    }

    template<bool dict>
    typename VecEnvWrapper<dict>::StepT
    VecFrameStack<dict>::step(int index, torch::Tensor const &action) noexcept {
        auto stepData = this->venv->step(index, action);
        stepData.observation = stackedObservation.update(stepData.observation);
        return stepData;
    }

    template<bool dict>
    std::unique_ptr<VecEnv<dict>> VecFrameStack<dict>::make(std::unique_ptr<VecEnv<dict>> x,
                                                            int nStack,
                                                            std::optional<std::string> const &channelOrder) {
        return std::make_unique<VecFrameStack<dict>>(std::move(x), nStack, channelOrder);
    }

    template class VecFrameStack<true>;
    template class VecFrameStack<false>;
}