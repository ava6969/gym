//
// Created by dewe on 12/28/21.
//

#ifndef GYM_VEC_ATARI_H
#define GYM_VEC_ATARI_H

#include "wrappers/vec_env/vec_env_wrapper.h"
#include "common/utils.h"

namespace gym {


    template<class VenvT, bool dict>
    class VecNormAndPermute : public VecEnvWrapper<VenvT, dict> {
        std::vector<std::string> key;
    public:
        explicit VecNormAndPermute(std::unique_ptr <VenvT> env) : VecEnvWrapper<VenvT, dict> ( std::move(env)){
            this->m_ActionSpace = this->actionSpace();
            auto _space = this->observationSpace();

            if( ADict* ptr = _space->template as<ADict>()){
                auto namedSpaces =  ptr->namedSpaces();
                NamedSpaces _newSpaces;

                for(auto const& [name, space_] : namedSpaces ){
                    if(space_->size().size() == 3){
                        std::vector<int64_t> sz = space_->size();
                        auto channel = sz.back();
                        sz[2] = sz[0];
                        sz[0] = channel;
                        key.push_back(name);
                        _newSpaces[name] = gym::makeBoxSpace<float>(0, 1, sz );
                    }else if(space_->size().size() == 4){
                        std::vector<int64_t> sz = space_->size();
                        auto channel = sz.back();
                        sz[3] = sz[1];
                        sz[1] = channel;
                        key.push_back(name);
                        _newSpaces[name] = gym::makeBoxSpace<float>(0, 1, sz );
                    }else{
                        _newSpaces[name] = space_->clone();
                    }
                }

                this->m_ObservationSpace = gym::makeDictionarySpace( std::move(_newSpaces) );
            }else{
                auto sz = this->observationSpace()->size();
                auto channel = sz.back();
                sz[2] = sz[0];
                sz[0] = channel;
                this->m_ObservationSpace = gym::makeBoxSpace<float>(0, 1, sz );
            }
        }

        explicit VecNormAndPermute(std::shared_ptr <Space> obsSpace, std::shared_ptr <Space> actSpace) :
                VecEnvWrapper<VenvT, false>(std::move(obsSpace), std::move(actSpace)) {}

        [[nodiscard]] inline torch::Tensor observation(torch::Tensor const &x) const noexcept {
            return x.dim() == 4 ? x.permute({0, 3, 1, 2}) / 255 : x.permute({0, 1, 4, 2, 3}) / 255;
        }

        [[nodiscard]] inline auto observation(::TensorDict x) const noexcept {
            for(auto& k : key)
                x[k] = x[k].dim() == 4 ? x[k].permute({0, 3, 1, 2}) / 255 : x[k].permute({0, 1, 4, 2, 3}) / 255;
            return x;
        }

        inline infer_type<dict> reset() noexcept override {
            return observation( this->venv->reset());
        }

        inline infer_type<dict> reset(int index) override {
            auto obs = this->venv->reset(index);
            if constexpr(dict){
                for(auto& k : key)
                    obs[k] = obs[k].unsqueeze(0).permute({0, 3, 1, 2}) / 255;
                return obs;
            }else{
                return observation( obs.unsqueeze(0));
            }
        }

        typename VenvT::StepT stepWait() noexcept override {
            auto stepT = this->venv->stepWait();
            stepT.observation = observation(stepT.observation);
            return stepT;
        }

        typename VenvT::StepT step(int index, torch::Tensor const& action) noexcept override{
            auto stepData = this->venv->step(index, action);
            if constexpr(dict){
                for(auto& [k, v] : stepData.observation)
                    v = v.unsqueeze(0).permute({0, 3, 1, 2}) / 255;
            }else{
                stepData.observation = observation(stepData.observation.unsqueeze(0));
            }

            return stepData;
        }

        static auto make(std::unique_ptr<VenvT> x){
            return std::make_unique<VecNormAndPermute<VenvT, dict>>( std::move(x) );
        }
    };
}

#endif //GYM_VEC_ATARI_H
