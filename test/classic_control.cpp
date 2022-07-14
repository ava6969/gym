//
// Created by dewe on 8/22/21.
//


#include"gym_.h"
#include "classic_control/mountain_car.h"
#include "catch.hpp"
#include "classic_control/cartpole.h"
#include "chrono"
#include "python_gym/python_env.h"
#include "thread"
#include "wrappers/time_limit.h"

#define PY_ENV_ID "CartPole-v1"

//template<class EnvT, size_t N, size_t N_env>
//void test_run(std::string const& id, gym::Kwargs const& arg=std::nullopt){
//
//    using MonitorT = gym::MonitorWithEarlyReset<EnvT>;
//    using VecMonitorT = gym::SyncVecEnv<MonitorT, false>;
//
//    std::vector< std::unique_ptr<MonitorT> > v;
//
//    for(int i =0;i < N_env; i++)
//        v.template emplace_back( std::make_unique< MonitorT >( std::make_unique<EnvT>(), id, "log.txt") );
//
//    std::unique_ptr<gym::VecEnv<false>> env = std::make_unique<gym::TrainingVecNormRO<VecMonitorT>>(
//            std::make_unique<VecMonitorT>( std::move(v) ) );
//    auto n = env->actionSpace()->template as< gym::Discrete >()->n;
//
//    auto obs = env->reset();
//    int i = 0;
//    while (i < N){
//        auto stepResponse = env->step(  torch::randint(n, N_env) );
////        env->render(gym::RenderType::HUMAN);
//        i += N_env;
//    }
//}
//
//template<class EnvT, class actionT, size_t N>
//void test_run( gym::Kwargs const& args=std::nullopt ){
//
//    auto env = std::make_unique< gym::TimeLimit<EnvT> >( std::make_unique<EnvT>(args), std::nullopt, std::nullopt );
//
//    for(size_t i = 0; i < N; i++){
//        auto obs = env->reset();
//        typename EnvT::StepT stepResponse;
//
//        stepResponse.done = false;
//        std::any info;
//
//        int steps = 0 ;
//        while (not stepResponse.done){
//            stepResponse = env->step( env->actionSpace()->template sample< actionT >() );
//
//            env->render(gym::RenderType::HUMAN);
//
//            steps++;
//        }
//
//        std::cout << steps << "\n";
//    }
//}
//
//template<class VecEnvT, size_t N, size_t N_env>
//void p_test_run(gym::Kwargs const& args=std::nullopt){
//
//    using EnvT = typename VecEnvT::EnvT;
//
//    std::vector< std::unique_ptr<EnvT> > v;
//
//    for(int i =0;i < N_env; i++)
//        v.template emplace_back( std::make_unique<EnvT>(args) );
//
//    gym::VecEnv<false>* env = new VecEnvT(std::move(v) );
//
//    auto n = env->actionSpace()->template as< gym::Discrete >()->n;
//
//    auto obs = env->reset();
//    bool done = false;
//    assert( obs.size(0) == N_env);
//    assert( obs.size(1) == 4);
//    std::any info;
//
//    int steps = 0 ;
//    int i = 0;
//    auto start = std::chrono::high_resolution_clock::now();
//    while (i < N){
//        auto stepResponse = env->step( torch::randint(n, N_env) );
////        env->render(gym::RenderType::HUMAN);
//        i += N_env;
//    }
//    std::cout << std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count() << " s\n";
//    delete env;
//}

TEST_CASE("CartPole Response"){
    gym::CartPoleEnv env;
    env.seed(1);

    auto obs = env.reset();
    std::vector< int > actions{
            0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0
    };
    std::vector< bool > dones{
            false, false, false, false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false, true
    };
    std::vector< std::vector<double> > _obs{
            std::vector<double>{ 0.03076804, -0.19321569, -0.03151444,  0.25146705},
            std::vector<double>{ 0.02690373,  0.00234178, -0.02648509, -0.0509872 },
            std::vector<double>{ 0.02695057,  0.19783329, -0.02750484, -0.35190733},
            std::vector<double>{ 0.03090723,  0.00311305, -0.03454298, -0.06802285},
            std::vector<double>{ 0.03096949, -0.19149707, -0.03590344,  0.21356457},
            std::vector<double>{ 0.02713955, -0.38608781, -0.03163215,  0.49470922},
            std::vector<double>{ 0.01941779, -0.19053437, -0.02173797,  0.1922275 },
            std::vector<double>{ 0.01560711, -0.38533872, -0.01789342,  0.47797452},
            std::vector<double>{ 0.00790033, -0.18996877, -0.00833393,  0.17970613},
            std::vector<double>{ 0.00410096, -0.38497047, -0.0047398 ,  0.46974839},
            std::vector<double>{-0.00359845, -0.58002515,  0.00465516,  0.76093362},
            std::vector<double>{-0.01519895, -0.38496764,  0.01987384,  0.46971914},
            std::vector<double>{-0.02289831, -0.58036461,  0.02926822,  0.76859924},
            std::vector<double>{-0.0345056 , -0.77587695,  0.0446402 ,  1.07034578},
            std::vector<double>{-0.05002314, -0.97155987,  0.06604712,  1.3766976 },
            std::vector<double>{-0.06945434, -0.77732225,  0.09358107,  1.10538016},
            std::vector<double>{-0.08500078, -0.97354171,  0.11568868,  1.4258942 },
            std::vector<double>{-0.10447162, -1.16988765,  0.14420656,  1.75238106},
            std::vector<double>{-0.12786937, -1.36632181,  0.17925418,  2.08622383},
            std::vector<double>{-0.1551958 , -1.56274584,  0.22097866,  2.42855786}
    };

    REQUIRE_THAT(obs,
                 Catch::Matchers::Approx( std::vector<double>{0.03073904,  0.00145001, -0.03088818, -0.03131252}).epsilon(0.001)  );
    for (int i = 0; i < dones.size(); i++) {
        auto response = env.step( actions[i] );
        REQUIRE_THAT(response.observation, Catch::Matchers::Approx(_obs[i]).epsilon(0.001) );
        REQUIRE(response.reward == 1.f );
        REQUIRE(response.done ==  dones[i]);
    }

}


TEST_CASE("Mountain Response"){
    gym::MountainCarEnv env;
    env.seed(1);

    auto obs = env.reset();
    std::vector< int > actions{
            1, 1, 0, 0, 1, 1, 0, 1, 2, 1
    };

    std::vector< std::vector<double> > _obs{
            std::vector<double>{ -0.43915308, -0.00063117},
            std::vector<double>{ -0.44041085, -0.00125776 },
            std::vector<double>{ -0.44328606, -0.00287521},
            std::vector<double>{ -0.44775781, -0.00447175},
            std::vector<double>{ -0.45279347, -0.00503566},
            std::vector<double>{ -0.45835619, -0.00556272},
            std::vector<double>{ -0.4654051 , -0.00704892 },
            std::vector<double>{ -0.47288826, -0.00748316 },
            std::vector<double>{ -0.47975028, -0.00686202 },
            std::vector<double>{ -0.48694022, -0.00718994 },
    };

    REQUIRE_THAT(obs,
                 Catch::Matchers::Approx( std::vector<double>{-0.43852191,  0. }).epsilon(0.001)  );
    for (int i = 0; i < actions.size(); i++) {
        auto response = env.step( actions[i] );
        REQUIRE_THAT(response.observation, Catch::Matchers::Approx(_obs[i]).epsilon(0.001) );
        REQUIRE(response.reward == -1.0 );
        REQUIRE(response.done ==  false);
    }

}



//TEST_CASE("Running CartPole"){
//    test_run<gym::CartPoleEnv, int, 20>();
//}
//
//TEST_CASE("Running CartPole With Monitor && Norm"){
//    test_run<gym::CartPoleEnv, 20000, 10>("CartPole");
//}
//
//TEST_CASE("Running CartPole With [A]SyncVecEnv"){
//    p_test_run<gym::ASyncVecEnv<gym::CartPoleEnv, false>, 1000000, 1028>();
//    p_test_run<gym::SyncVecEnv<gym::CartPoleEnv, false>, 1000000, 1028>();
//}
//
//TEST_CASE("Running Py::CartPole"){
//    test_run<gym::PythonEnv<false, false>, int, 20>( gym::ArgMap{{"id", std::string("CartPole-v1") } } );
//}
//
//TEST_CASE("Running Py::LunarLander"){
//    test_run<gym::PythonEnv<false, false>, int, 20>( gym::ArgMap{{"id", std::string("LunarLander-v2") } } );
//}

//TEST_CASE("Running MountainCarEnv") {
//    test_run<gym::MountainCarEnv, int, 20>();
//}