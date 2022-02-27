//
// Created by dewe on 12/17/21.
//

#ifndef GYM_SYNC_ENV_H
#define GYM_SYNC_ENV_H

#include "base.h"
#include "common/tensor_adapter.h"
namespace gym{

    template<bool dict>
    using infer_t_v = std::conditional_t<!dict, std::vector<torch::Tensor>,
            std::unordered_map<std::string, std::vector<torch::Tensor>>>;

    template<class EnvType, bool dict=false>
    class SyncVecEnv : public VecEnv<dict>{

    public:

        using EnvT = EnvType;

        static auto make(std::vector< std::unique_ptr<
                Env<typename EnvType::ObservationT, typename EnvType::ActionT>>> env){
            return std::make_unique<SyncVecEnv<EnvType, dict>>( std::move(env) );
        }

        SyncVecEnv(std::shared_ptr<Space> o_space,
                   std::shared_ptr<Space> a_space,
                   std::vector< std::unique_ptr<
                   Env<typename EnvType::ObservationT, typename EnvType::ActionT>>> envs): VecEnv<dict>( envs.size(),
                                                                       std::move(o_space),
                                                                       std::move(a_space)),
                                                                       envs( std::move( envs ) ){
            m_BufRews .resize( this->numEnvs );
            m_BufDones.resize( this->numEnvs );
            m_BufInfos.resize( this->numEnvs );
            m_Actions.resize( this->numEnvs );
            if constexpr(dict){
                for(auto const& [k, _] : o_space->namedSpaces())
                    m_BufObs[k].resize(this->numEnvs );
            }else
                m_BufObs.resize(this->numEnvs );
        }

        explicit SyncVecEnv(std::vector< std::unique_ptr<
                Env<typename EnvType::ObservationT, typename EnvType::ActionT>>> env ):
        VecEnv<dict>( env.size(), std::move(env[0]->observationSpace()), std::move(env[0]->actionSpace())),
                envs( std::move( env ) ){
            m_BufRews .resize( this->numEnvs );
            m_BufDones.resize( this->numEnvs );
            m_BufInfos.resize( this->numEnvs );
            m_Actions.resize( this->numEnvs );
            if constexpr(dict){
                for(auto const& [k, _] : this->m_ObservationSpace->namedSpaces())
                    m_BufObs[k].resize(this->numEnvs );
            }else
                m_BufObs.resize(this->numEnvs );
        }

        inline typename VecEnv<dict>::ObservationT reset() noexcept override{
            int i = 0;
            for (auto& env: envs) {
                saveObs(i++, std::move( env->reset() ));
            }
            return mergeObs();
        }

        inline std::vector<size_t> seed( size_t _seed) noexcept override {
            std::vector<size_t> seeds;
            size_t idx = 0;
            for (auto& env : this->envs){
                env->seed(_seed + idx);
                seeds.push_back( _seed + idx );
                idx++;
            }
            return seeds;
        }

        inline void stepAsync(torch::Tensor const& _actions) noexcept override{
            TensorAdapter::decode(_actions, m_Actions);
        }

        inline void stepPerWorker( std::unique_ptr<Env<typename EnvType::ObservationT, typename EnvType::ActionT>>& env,
                                   int rank)  noexcept{
            auto response = env->step( m_Actions[rank] );
            if( response.done ){
                if(response.info.contains("episode")){
                    if( std::any_cast<Result>(response.info["episode"]).l == 1){
                        printf("\n");
                    }
                }

                response.observation = env->reset();
            }
            m_BufInfos[rank] = std::move( response.info );
            m_BufDones[rank] = static_cast<float>(response.done) ;
            m_BufRews[rank] = std::move( response.reward );
            saveObs(rank, std::move( response.observation ) );
        }

        inline typename VecEnv<dict>::StepT complete(){
            return  { this->mergeObs(),
                      torch::tensor( this->m_BufRews ).unsqueeze(-1),
                      torch::tensor( this->m_BufDones).unsqueeze(-1),
                      m_BufInfos };
        }

        typename  VecEnv<dict>::StepT stepWait() noexcept override{
            int i = 0;
            clearedInfo = false;

            for(auto& env: envs){
                stepPerWorker(env, i++);
            }
            return complete();
        }
        virtual void render() const override{
            this->envs[0]->render(RenderType::HUMAN);
        }

        virtual typename VecEnv<dict>::ObservationT reset(int index){
            saveObs(index, this->envs[index]->reset());
            if constexpr(! dict)
                return m_BufObs[index];
            else{
                TensorDict x;
                for (auto const& [k, v]: m_BufObs ) {
                    x[k] = v[index];
                }
                return x;
            }
        }

        inline typename VecEnv<dict>::StepT step(int index, torch::Tensor const& action) noexcept override{
            m_Actions[index].resize(action.size(0));
            TensorAdapter::decode< typename EnvType::ActionT >(action, m_Actions[index]);
            stepPerWorker(envs[index], index);
            infer_type<dict> out;
            if constexpr(dict){
                for (auto const& [k, v]: m_BufObs ) {
                    out[k] = v[index];
                }
            } else
                out = m_BufObs[index];

            return { out,
                     torch::tensor( this->m_BufRews[index] ).unsqueeze(-1),
                     torch::tensor( this->m_BufDones[index] ).unsqueeze(-1),
                     {m_BufInfos[index]} };
        }

    protected:
        mutable std::vector< std::unique_ptr< Env<typename EnvType::ObservationT, typename EnvType::ActionT>>> envs;
        std::vector<typename EnvType::ActionT > m_Actions;
        infer_t_v<dict> m_BufObs{};
        std::vector<int> completedWorkers;
        std::vector<float> m_BufRews{};
        std::vector<float> m_BufDones{};
        std::vector<AnyMap> m_BufInfos{};
        bool clearedInfo{false};

        inline void saveObs(int env_idx, typename EnvType::ObservationT && _obs ) {
            if constexpr(dict) {
                for (auto const &key: this->m_ObservationSpace->keys()){
                    this->m_BufObs[key][env_idx] = TensorAdapter::encode( std::move(_obs[key]) );
                }
            } else {
                this->m_BufObs[env_idx] = TensorAdapter::encode( std::move(_obs) );
            }
        }

        inline typename VecEnv<dict>::ObservationT  mergeObs( ) {
            if constexpr(dict) {
                TensorDict x;
                for(auto const& k : this->m_ObservationSpace->keys())
                    x[k] = torch::stack(m_BufObs[k]);
                return x;
            } else {
                return torch::stack(m_BufObs);
            }
        }
    };
}
#endif //GYM_SYNC_ENV_H
