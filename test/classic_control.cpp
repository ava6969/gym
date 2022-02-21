//
// Created by dewe on 8/22/21.
//

#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_MAIN
#endif

#include "gym_.h"
#include "classic_control/mountain_car.h"
#include "catch.hpp"
#include "classic_control/cartpole.h"
#include "chrono"
#include "python_gym/python_env.h"
#include "thread"
#include "wrappers/time_limit.h"

#define PY_ENV_ID "CartPole-v1"

template<class EnvT, size_t N, size_t N_env>
void test_run(std::string const& id, gym::OptionalArgMap const& arg=std::nullopt){

    using MonitorT = gym::MonitorWithEarlyReset<EnvT>;
    using VecMonitorT = gym::SyncVecEnv<MonitorT>;

    std::vector< std::unique_ptr<MonitorT> > v;

    for(int i =0;i < N_env; i++)
        v.template emplace_back( std::make_unique< MonitorT >( std::make_unique<EnvT>(arg), id, "log.txt") );

    std::unique_ptr<gym::VecEnv<false>> env = std::make_unique<gym::TrainingVecNormRO<VecMonitorT>>(
            std::make_unique<VecMonitorT>( std::move(v) ) );
    auto n = env->actionSpace()->template as< gym::Discrete >()->n;

    auto obs = env->reset();
    int i = 0;
    while (i < N){
        auto stepResponse = env->step(  torch::randint(n, N_env) );
//        env->render(gym::RenderType::HUMAN);
        i += N_env;
    }
}

template<class EnvT, class actionT, size_t N>
void test_run( gym::OptionalArgMap const& args=std::nullopt ){

    auto env = std::make_unique< gym::TimeLimit<EnvT> >( std::make_unique<EnvT>(args), std::nullopt, std::nullopt );

    for(size_t i = 0; i < N; i++){
        auto obs = env->reset();
        typename EnvT::StepT stepResponse;

        stepResponse.done = false;
        std::any info;

        int steps = 0 ;
        while (not stepResponse.done){
            stepResponse = env->step( env->actionSpace()->template sample< actionT >() );

            env->render(gym::RenderType::HUMAN);

            steps++;
        }

        std::cout << steps << "\n";
    }
}

template<class VecEnvT, size_t N, size_t N_env>
void p_test_run(gym::OptionalArgMap const& args=std::nullopt){

    using EnvT = typename VecEnvT::EnvT;

    std::vector< std::unique_ptr<EnvT> > v;

    for(int i =0;i < N_env; i++)
        v.template emplace_back( std::make_unique<EnvT>(args) );

    gym::VecEnv<false>* env = new VecEnvT(std::move(v) );

    auto n = env->actionSpace()->template as< gym::Discrete >()->n;

    auto obs = env->reset();
    bool done = false;
    assert( obs.size(0) == N_env);
    assert( obs.size(1) == 4);
    std::any info;

    int steps = 0 ;
    int i = 0;
    auto start = std::chrono::high_resolution_clock::now();
    while (i < N){
        auto stepResponse = env->step( torch::randint(n, N_env) );
//        env->render(gym::RenderType::HUMAN);
        i += N_env;
    }
    std::cout << std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count() << " s\n";
    delete env;
}

TEST_CASE("Running CartPole"){
    test_run<gym::CartPoleEnv, int, 20>();
}

TEST_CASE("Running CartPole With Monitor && Norm"){
    test_run<gym::CartPoleEnv, 20000, 10>("CartPole");
}

TEST_CASE("Running CartPole With [A]SyncVecEnv"){
    p_test_run<gym::ASyncVecEnv<gym::CartPoleEnv, false>, 1000000, 1028>();
    p_test_run<gym::SyncVecEnv<gym::CartPoleEnv, false>, 1000000, 1028>();
}

TEST_CASE("Running Py::CartPole"){
    test_run<gym::PythonEnv<false, false>, int, 20>( gym::ArgMap{{"id", std::string("CartPole-v1") } } );
}

TEST_CASE("Running Py::LunarLander"){
    test_run<gym::PythonEnv<false, false>, int, 20>( gym::ArgMap{{"id", std::string("LunarLander-v2") } } );
}

//TEST_CASE("Running MountainCarEnv") {
//    test_run<gym::MountainCarEnv, int, 20>();
//}