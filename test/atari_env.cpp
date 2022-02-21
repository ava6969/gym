//
// Created by dewe on 8/31/21.
//

#include <python_gym/python_env.h>
#include "catch.hpp"
#include "wrappers/vec_env/vec_atari.h"
#include "wrappers/vec_env/sync_env.h"

//TEST_CASE("Air Raid"){
//
//    gym::AtariEnv<true> env("air_raid");
//
//    for(int i =0; i < 5; i++){
//
//        env.reset();
//        float t{};
//        while(true){
//            auto[obs, reward, done, info] = env.step(env.actionSpace()->sample<int>());
//            env.render(gym::RenderType::HUMAN);
//
//            t += reward;
//
//            if(done){
//                std::cout << std::any_cast<int>(info.at("ale.lives")) << "\n";
//                std::cout << t << "\n";
//                break;
//            }
//        }
//    }
//}

TEST_CASE("Py Air Raid"){

    gym::ArgMap args;
    args["id"] = std::string("Atari::PongNoFrameskip-v4");

    auto env = std::make_unique<gym::PythonEnv<>>(args);

    for(int i =0; i < 5; i++){

        env->reset();
        float t{};
        while(true){

            auto[obs, reward, done, info] = env->step(env->actionSpace()->sample<int>());
            env->render(gym::RenderType::HUMAN);

            t += reward;

            if(done){
                std::cout << t << "\n";
                break;
            }
        }
    }
}

TEST_CASE("Atari Processing"){

    gym::ArgMap args;
    args["id"] = std::string("Atari::AirRaid-v0");
    using EnvT = gym::PythonEnv<false, false>;
    using SyncT = gym::SyncVecEnv< EnvT , false>;

    std::vector< std::unique_ptr< EnvT > > envs;
    envs.emplace_back( std::make_unique< EnvT >( args ) );
    auto sync_env = std::make_unique< SyncT >( std::move(envs) );
    std::unique_ptr< gym::VecEnv<false> > atariProcessing =
            std::make_unique< gym::VecNormAndPermute<SyncT> >(std::move(sync_env) );

    auto space = atariProcessing->observationSpace();
    for(int i =0; i < 5; i++){

        auto obs = atariProcessing->reset();
        std::cout << obs << "\n";
        float t{};
        while(true){
            auto[obs, reward, done, info] =
                    atariProcessing->step( torch::randint( atariProcessing->actionSpace()->size()[0], {1} ) );

            std::cout << obs.sizes() << "\n";
            atariProcessing->render();

            t += reward.item<float>();

            if(done.item<bool>()){
                std::cout << t << "\n";
                break;
            }
        }
    }
}