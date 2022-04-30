//
// Created by dewe on 3/28/22.
//

#ifndef GYM_VEC_FRAME_SKIP_H
#define GYM_VEC_FRAME_SKIP_H


#include "wrappers/vec_env/vec_env_wrapper.h"
#include "common/utils.h"

namespace gym {


    template<bool dict, bool repeatAction>
    class VecFrameSkip : public VecEnvWrapper<dict> {
        int m_FramesToSkip{};
        int m_FramesSkipped{};
        typename VecEnv<dict>::StepT m_LastStep{};

    public:
        explicit VecFrameSkip(std::unique_ptr < VecEnv<dict> > env, int frames_to_skip) :
        VecEnvWrapper<dict> (
                std::move(env)),
                m_FramesToSkip(frames_to_skip),
                m_FramesSkipped(0) {}

        inline infer_type<dict> reset() noexcept override {
            m_FramesSkipped = 0;
            return this->venv->reset();
        }

        inline infer_type<dict> reset(int index) override {
            m_FramesSkipped = 0;
            return this->venv->reset(index);
        }

        typename VecEnv<dict>::StepT stepWait() noexcept override {
            if constexpr(repeatAction){
                return this->venv->stepWait();
            }

            if(m_FramesSkipped - 1 == 0){
                m_LastStep = this->venv->stepWait();
            }

            return m_LastStep;
        }

        inline void stepAsync(torch::Tensor const& _actions) noexcept override{
            if(m_FramesSkipped == m_FramesToSkip){
                this->venv->stepAsync(_actions);
                m_FramesSkipped = 0;
            }else{
                m_FramesSkipped++;
            }
        }

//        typename VenvT::StepT step(int index, torch::Tensor const& action) noexcept override{
//            if(m_FramesSkipped == m_FramesToSkip){
//                this->venv->stepAsync(_actions);
//                m_FramesSkipped = 0;
//            }else{
//                m_FramesSkipped++;
//            }
//
//            auto stepData = this->venv->step(index, action);
//            if constexpr(dict){
//                for(auto& [k, v] : stepData.observation)
//                    v = v.unsqueeze(0).permute({0, 3, 1, 2}) / 255;
//            }else{
//                stepData.observation = observation(stepData.observation.unsqueeze(0));
//            }
//
//            return stepData;
//        }

        static auto make(std::unique_ptr<VecEnv<dict>> x, int frames_to_skip){
            return std::make_unique<VecFrameSkip<dict, repeatAction>>( std::move(x), frames_to_skip );
        }
    };
}

#endif //GYM_VEC_FRAME_SKIP_H
