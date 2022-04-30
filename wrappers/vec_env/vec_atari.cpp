//
// Created by dewe on 12/28/21.
//

#include "spaces/space.h"
#include "spaces/dict.h"
#include "common/utils.h"
#include "vec_atari.h"

namespace gym {

        template<bool dict>
        VecNormAndPermute<dict>::VecNormAndPermute(std::unique_ptr <VecEnv<dict>> env)
        : VecEnvWrapper< dict> ( std::move(env) ){

            this->m_ActionSpace = this->actionSpace();
            auto _space = this->observationSpace();

            if( ADict* ptr = _space->template as<ADict>()){
                auto namedSpaces =  ptr->namedSpaces();
                NamedSpaces _newSpaces;

                for(auto const& [name, space_] : namedSpaces ){
                    std::vector<int64_t> sz;
                    if(space_->size().size() == 3){
                        sz = space_->size();
                        auto channel = sz.back();
                        sz[2] = sz[0];
                        sz[0] = channel;
                        key.push_back(name);
                        _newSpaces[name] = gym::makeBoxSpace<float>(0, 1, sz );
                    }else if(space_->size().size() == 4){
                        sz = space_->size();
                        auto channel = sz.back();
                        sz[3] = sz[1];
                        sz[1] = channel;
                        key.push_back(name);
                    }else{
                        _newSpaces[name] = space_->clone();
                        continue;
                    }

                    if(auto sh = space_->template as<Box<float>>()){
                        norm = false;
                        _newSpaces[name] = gym::makeBoxSpace<float>(sh->getRange()[0].low,
                                                                    sh->getRange()[0].high, sz );
                    }else{
                        _newSpaces[name] = gym::makeBoxSpace<float>(0, 1, sz );
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

    template<bool dict>
    VecNormAndPermute<dict>::VecNormAndPermute(std::unique_ptr <VecEnv<dict>> env,
                                               std::shared_ptr <Space> obsSpace,
                                               std::shared_ptr <Space> actSpace) :
                VecEnvWrapper<dict>(std::move(env),
                                    std::move(obsSpace),
                                    std::move(actSpace)) {}

    template<bool dict> [[nodiscard]]  torch::Tensor VecNormAndPermute<dict>::observation(torch::Tensor const &x)
    const noexcept
    {
        auto y = x.dim() == 4 ? x.permute({0, 3, 1, 2}) : x.permute({0, 1, 4, 2, 3});
        if (norm)
            y = y / 255;
        else {
            y = y.to(torch::kFloat);
        }
        return y;
    }


    template<bool dict> [[nodiscard]] auto VecNormAndPermute<dict>::observation(TensorDict x) const noexcept {
        for (auto &k: key)
            x[k] = observation(x[k]);
        return x;
    }

    template<bool dict> typename VecEnv<dict>::ObservationT VecNormAndPermute<dict>::reset() noexcept {
        return observation(this->venv->reset());
    }

    template<bool dict> typename VecEnv<dict>::ObservationT VecNormAndPermute<dict>::reset(int index) {
        auto obs = this->venv->reset(index);
        if constexpr(dict) {
            for (auto &k: key)
                obs[k] = obs[k].unsqueeze(0).permute({0, 3, 1, 2}) / 255;
            return obs;
        } else {
            return observation(obs.unsqueeze(0));
        }
    }

    template<bool dict> typename VecEnv<dict>::StepT VecNormAndPermute<dict>::stepWait() noexcept {
        auto stepT = this->venv->stepWait();
        stepT.observation = observation(stepT.observation);
        return stepT;
    }

    template<bool dict> typename VecEnv<dict> ::StepT VecNormAndPermute<dict>::step(int index, torch::Tensor const& action) noexcept {
        auto stepData = this->venv->step(index, action);
        if constexpr(dict) {
            for (auto &[k, v]: stepData.observation)
                v = v.unsqueeze(0).permute({0, 3, 1, 2}) / 255;
        } else {
            stepData.observation = observation(stepData.observation.unsqueeze(0));
        }

        return stepData;
    }

    template<bool dict>  std::unique_ptr< VecEnv<dict> > VecNormAndPermute<dict>::make(std::unique_ptr< VecEnv<dict> > x) {
        return std::make_unique< VecNormAndPermute<dict> >(std::move(x));
    }

    template class VecNormAndPermute<true>;
    template class VecNormAndPermute<false>;

}
