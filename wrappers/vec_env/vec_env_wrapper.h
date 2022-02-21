//
// Created by dewe on 12/17/21.
//

#ifndef GYM_VEC_ENV_WRAPPER_H
#define GYM_VEC_ENV_WRAPPER_H

#include "base.h"

namespace gym {

    template<typename VenvType, bool dict>
    class VecEnvWrapper : public VecEnv<dict> {

    public:
        VecEnvWrapper()=default;
        explicit VecEnvWrapper( std::unique_ptr< VenvType > venv,
                      std::shared_ptr<Space> o_space=nullptr,
                      std::shared_ptr<Space> a_space=nullptr):VecEnv<dict>( venv->nEnvs(),
                                                                       std::move(o_space),
                                                                       std::move(a_space)){
            this->venv = (std::move(venv));
            if(not this->m_ObservationSpace)
                this->m_ObservationSpace = this->venv->observationSpace();
            if(not this->m_ActionSpace)
                this->m_ActionSpace = this->venv->actionSpace();
        }

        inline typename VenvType::ObservationT reset() noexcept override{
            return venv->reset();
        }

       inline void stepAsync( torch::Tensor const& actions) noexcept override{
            this->venv->stepAsync( actions );
        }

        virtual void render() const { this->venv->render(); }

        typename VenvType::StepT stepWait() noexcept override{
            return venv->stepWait();
        }

        inline std::vector<size_t> seed( size_t _seed) noexcept override {
            return venv->seed( _seed );
        }

        virtual typename VenvType::ObservationT reset(int index) { return venv->reset(index); }

        inline typename VenvType::StepT step(int index, torch::Tensor const& action) noexcept{
            return venv->step(index, action);
        }

    protected:
        std::unique_ptr< VenvType > venv{nullptr};
    };
}

#endif //GYM_VEC_ENV_WRAPPER_H
