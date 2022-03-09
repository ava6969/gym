//
// Created by dewe on 12/17/21.
//

#pragma once

#include "sync_env.h"
#include "barrier"

namespace gym {

    template<class EnvType, bool dict=false>
    struct Worker;

    template<class EnvType, bool dict>
    class ASyncVecEnv : public SyncVecEnv<EnvType, dict> {

    public:
        using EnvT = EnvType;

        ASyncVecEnv(std::vector< std::unique_ptr<
                Env<typename EnvType::ObservationT, typename EnvType::ActionT>>> env,
                    int max_threads=std::thread::hardware_concurrency()):
        SyncVecEnv<EnvType, dict>( std::move(env)),
                stop(false){

            if(this->numEnvs <= max_threads){
                maxThreads = this->numEnvs;
                init_barrier = std::make_unique<std::barrier<>>( this->numEnvs + 1 );
                env_barrier = std::make_unique<std::barrier<>>( this->numEnvs + 1 );
                allWaiting.assign(this->numEnvs, true);
                for(int rank =0; rank < this->numEnvs; rank++){
                    workers.template emplace_back( [this, rank](){
                        auto& env = this->envs.at(rank);
                        allWaiting[rank] = false;

                        init_barrier->arrive_and_wait();
                        this->saveObs(rank, std::move( env->reset() ));
                        auto t = env_barrier->arrive(1);

                        while (! this->stop ){
                            init_barrier->arrive_and_wait();
                            if(this->cantStep)
                                this->stepPerWorker(env, rank);
                            t = env_barrier->arrive(1);
                        }
                    });
                }
            } else{
                maxThreads = std::min<int>(max_threads != 0 ? max_threads : 2,
                                            std::thread::hardware_concurrency());
                auto block_size = std::ceil(this->numEnvs / maxThreads );
                allWaiting.assign(maxThreads, true);
                int i = 0;
                init_barrier = std::make_unique<std::barrier<>>( this->maxThreads + 1 );
                env_barrier = std::make_unique<std::barrier<>>( this->maxThreads + 1 );
                int last_rank = 0;
                int rank = block_size;
                while (i < maxThreads){

                    workers.emplace_back( [this, last_rank, rank, i](){
                        int start_rank = last_rank;
                        auto [env_begin, env_end] = std::make_pair(this->envs.begin() + start_rank,this->envs.begin() + rank);
                        allWaiting[i] = false;

                        init_barrier->arrive_and_wait();
                        std::for_each(env_begin, env_end, [this, start_rank] (auto& env) mutable {
                            this->saveObs(start_rank++, std::move( env->reset() ));
                        });
                        auto t = env_barrier->arrive(1);

                        while (! this->stop ){
                            init_barrier->arrive_and_wait();
                            std::for_each(env_begin, env_end, [this, start_rank] (auto& env) mutable {
                                this->stepPerWorker(env, start_rank++);
                            });
                            t = env_barrier->arrive(1);
                        }
                    });
                    if(i + 2 == maxThreads)
                        block_size = this->numEnvs - ((i+1)*block_size);

                    last_rank = rank;
                    rank += block_size;
                    i++;
                }
            }

            while ( std::any_of(allWaiting.begin(), allWaiting.end(), [](auto v) { return v; }))
                std::this_thread::sleep_for( std::chrono::seconds(1) );
        }

        inline void stepAsync(torch::Tensor const& _actions) noexcept override{
            TensorAdapter::decode(_actions, this->m_Actions);
            cantStep = true;
            auto t = init_barrier->arrive(1);
            env_barrier->arrive_and_wait();
        }

        inline void release() noexcept {
            auto t = init_barrier->arrive(1);
            env_barrier->arrive_and_wait();
        }

        inline typename  VecEnv<dict>::StepT stepWait() noexcept override{
            return this->complete();
        }

        ~ASyncVecEnv(){
            this->stop = true;
            release();
            for(auto& ftr: workers){
                ftr.join();
            }
        }

        inline typename VecEnv<dict>::ObservationT reset() noexcept override{
            auto t = init_barrier->arrive(1);
            env_barrier->arrive_and_wait();
            return this->mergeObs();
        }

        virtual typename VecEnv<dict>::ObservationT reset(int index) override{
            return SyncVecEnv<EnvT, dict>::reset(index);
        }

    private:
        bool stop{false};
        std::vector<bool> allWaiting;
        std::vector< std::thread > workers;
        int maxThreads{};
        std::atomic_int64_t ctr{0};
        bool cantStep = false;

        std::unique_ptr<std::barrier<>> init_barrier, env_barrier;
    };

}
