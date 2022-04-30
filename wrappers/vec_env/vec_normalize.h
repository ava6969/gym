//
// Created by dewe on 10/17/21.
//
#pragma once

#include <filesystem>
#include "common/normalizers/running_mean_std.h"
#include "updateable.h"
#include "vec_env_wrapper.h"
#include "boost/algorithm/string.hpp"

namespace gym {

    template<bool dict>
    struct VecNormalizeStateRewardPair{
        infer_type<dict> state;
        torch::Tensor reward, discounts;
    };

    template<bool dict, bool isTraining, bool normReward, bool normObs>
    class VecNormalize : public VecEnvWrapper<dict>, public Updateable<VecNormalizeStateRewardPair<dict>> {
        public:
            explicit VecNormalize(std::unique_ptr< VecEnv<dict> > m_Env,
                                float clip_obs=10.f,
                                float clip_reward=10.f,
                                float gamma=0.99,
                                float epsilon=1e-8):VecEnvWrapper<dict>(std::move(m_Env)),
                                m_ClipObs(clip_obs),
                                m_ClipReward(clip_reward),
                                m_Epsilon(epsilon),
                                m_Gamma( torch::tensor(gamma) ),
                                ZERO(torch::zeros(this->numEnvs)){

                if constexpr(dict){
#ifdef EXCLUDES
                    std::vector<std::string> splitted;
                    boost::split(splitted, std::string(EXCLUDES), boost::is_any_of("_"));

                    auto filter = [&splitted](std::string const& x){
                        return std::ranges::none_of(splitted, [x](auto const& token){ return x.starts_with(token); });
                    };

                    for (auto const&[key, space]: this->observationSpace()->namedSpaces()) {
                        if( filter(key) )
                            m_ObsRMS[key] = RunningMeanStd( space->size() );
                    }
#else
                    for (auto const&[key, space]: this->observationSpace()->namedSpaces()) {
                        m_ObsRMS[key] = RunningMeanStd( space->size() );
                    }
#endif

                }else{
                    m_ObsRMS = RunningMeanStd( this->observationSpace()->size() );
                }

                m_RetRMS = RunningMeanStd( std::vector<int64_t>{} );
                m_Returns = ZERO;
            }

        explicit VecNormalize(VecEnv<dict>* main,
                              std::unique_ptr< VecEnv<dict> > m_Env,
                              float clip_obs=10.f,
                              float clip_reward=10.f,
                              float gamma=0.99,
                              float epsilon=1e-8):
                              VecNormalize( std::move(m_Env), clip_obs, clip_reward, gamma, epsilon){
                if(main){
                    if( auto normEnv = dynamic_cast<VecNormalize< dict, true, normReward, normObs>*>(main)){
                        std::tie(m_ObsRMS, m_RetRMS) = normEnv->get();
                    }
                }
        }

        inline auto get(){
               return std::make_pair(m_ObsRMS, m_RetRMS);
            }

            inline
            void save(std::filesystem::path const& path) const{
                if constexpr(normReward)
                    torch::save(m_RetRMS, (path / "returns.pt").string() );

                if constexpr(normObs){
                    if constexpr(dict){
                        for(auto const& [key, rms] : m_ObsRMS)
                            torch::save( rms, (path / key).concat(".pt").string());
                    }else{
                        torch::save( m_ObsRMS, (path / "observation.pt").string());
                    }
                }
            }

            inline auto oldObs() const { return old_obs; }

            inline
            void load(std::filesystem::path const& path) {
                if constexpr(normReward)
                    torch::load(m_RetRMS, (path / "returns.pt").string() );
                if constexpr(normObs) {
                    if constexpr(dict) {
                        for (auto&[key, rms]: m_ObsRMS)
                            torch::load(rms, (path / key).concat(".pt").string());
                    } else {
                        torch::load(m_ObsRMS, (path / "observation.pt").string());
                    }
                }
            }
        template<bool clone>
            inline void normalizeObs(TensorDict& obsDict) const noexcept{
                for(auto const& [k, rms] : m_ObsRMS){
                    auto const& obs = obsDict.at(k);
                    old_obs[k] = obs.clone();
                    obsDict[k] = torch::clip( (obs - rms->template mean<clone>()) / torch::sqrt( rms->template var<clone>() + m_Epsilon ),
                                              -m_ClipObs, m_ClipObs);
                }
            }
        template<bool clone>
            inline void normalizeObs(torch::Tensor& obs) const noexcept{
                old_obs = obs.clone();
                obs = torch::clip( (obs - m_ObsRMS->template mean<clone>()) /
                        torch::sqrt( m_ObsRMS->template var<clone>() + m_Epsilon ),
                                   -m_ClipObs, m_ClipObs);
            }

            template<bool clone>
            void normalizeReward(torch::Tensor& reward) const noexcept{
                reward = torch::clip( reward / torch::sqrt(m_RetRMS->var<clone>() + m_Epsilon), -m_ClipReward, m_ClipReward);
            }

            inline void updateReward(torch::Tensor const& reward) noexcept {
                m_Returns = m_Returns * m_Gamma + reward.squeeze(-1);
                m_RetRMS->update(m_Returns);
            }

        inline void updateObservation(infer_type<dict> const& _obs) {
            if constexpr(dict) {
                for (auto&[key, state]: m_ObsRMS)
                    state->update(_obs.at(key));
            } else {
                m_ObsRMS->update(_obs);
            }
        }

        infer_type<dict> reset() noexcept override {
            infer_type<dict> obs = this->venv->reset();
            m_Returns = ZERO;

            if constexpr(isTraining and normObs) {
                updateObservation(obs);
            }

            if constexpr(normObs) {
                normalizeObs<false>(obs);
            }
            return obs;
        }

        typename VecEnv<dict>::StepT stepWait() noexcept override{
            auto stepData = this->venv->stepWait();

            if constexpr(isTraining and normObs){
                updateObservation(stepData.observation);
            }

            if constexpr( normObs )
                normalizeObs<false>(stepData.observation);

            if constexpr( isTraining and normReward)
                updateReward( stepData.reward );

            if constexpr( normReward )
                normalizeReward<false>( stepData.reward );

            m_Returns = torch::where( stepData.done.squeeze(-1).toType(c10::kBool), ZERO, m_Returns);
            return stepData;
        }

        typename VecEnv<dict>::ObservationT reset(int index) noexcept override {
            typename VecEnv<dict>::ObservationT obs = this->venv->reset(index);

            m_Returns[index] = 0;

            if constexpr(normObs) {
                normalizeObs<true>(obs);
            }

            return obs;
        }

        void update(VecNormalizeStateRewardPair<dict> && x ) override {
                if constexpr(dict){
                    for(auto const& [k, _] : m_ObsRMS){
                        x.state[k] = x.state[k].cpu().flatten(0, 1);
                    }
                } else
                    x.state = x.state.cpu().flatten(0, 1);

            if constexpr(normObs) {
                updateObservation(x.state);
            }

            if constexpr(normReward){
                m_RetRMS->update( x.reward.clone().reshape( {-1, 1} ) );
            }
        }

        typename VecEnv<dict>::StepT step(int index, torch::Tensor const& action) noexcept override{
            auto stepData = this->venv->step(index, action);

            if constexpr(normObs)
                normalizeObs<true>(stepData.observation);

            if constexpr(normReward)
                normalizeReward<true>( stepData.reward );

            return stepData;
        }

        private:
            std::conditional_t<dict, std::unordered_map<std::string, RunningMeanStd> , RunningMeanStd> m_ObsRMS;
            mutable typename VecEnv<dict>::ObservationT old_obs;
            RunningMeanStd m_RetRMS;
            float m_ClipObs, m_ClipReward;
            float m_Epsilon;
            torch::Tensor const ZERO;
            torch::Tensor m_Returns;
            torch::Tensor m_Gamma;
            std::mutex returns_mtx;
            bool firstUpdate{true};
    };
}
