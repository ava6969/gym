//
// Created by dewe on 8/31/21.
//


#include "catch.hpp"
#include "atari/atari_env.h"
#include "wrappers/vec_env/vec_atari.h"
#include "wrappers/vec_env/sync_env.h"
#include "wrappers/vec_env/vec_frame_stack.h"
#include "wrappers/atari_wrappers.h"


using EnvT = gym::AtariEnv<true>;
using SyncT = gym::SyncVecEnv< gym::Env<cv::Mat, int>, false >;

TEST_CASE("Air Raid"){

    EnvT env("air_raid");

    for(int i =0; i < 5; i++){

        env.reset();
        float t{};
        while(true){
            auto[obs, reward, done, info] = env.step(env.actionSpace()->sample<int>());
            env.render(gym::RenderType::HUMAN);

            t += reward;

            if(done){
                std::cout << std::any_cast<int>(info.at("ale.lives")) << "\n";
                std::cout << t << "\n";
                break;
            }
        }
    }
}

TEST_CASE("Atari Processing"){

    std::vector< std::unique_ptr< gym::Env<cv::Mat, int> > > envs;
    envs.emplace_back(gym::AtariWrapper::make( std::make_unique< EnvT >( "air_raid" ) ) );
    envs.emplace_back(gym::AtariWrapper::make( std::make_unique< EnvT >( "air_raid" ) ) );
    envs.emplace_back(gym::AtariWrapper::make( std::make_unique< EnvT >( "air_raid" ) ) );
    envs.emplace_back(gym::AtariWrapper::make( std::make_unique< EnvT >( "air_raid" ) ) );

    auto sync_env = SyncT::make( std::move(envs), true );

    std::unique_ptr< gym::VecEnv<false> > atariProcessing =  gym::VecNormAndPermute<false>::make(
            gym::VecFrameStack<false>::make(std::move(sync_env) , 8) );

    auto space = atariProcessing->observationSpace();
    for(int i =0; i < 5; i++){

        std::cout << atariProcessing->reset() << "\n";
        float t{};
        while(true){
            auto[obs, reward, done, info] =
                    atariProcessing->step( torch::randint(atariProcessing->actionSpace()->size()[0], {4, 1}) );

//            std::cout << obs.sizes() << "\n";
            atariProcessing->render();

            t += reward[0].item<float>();

            if(done[0].item<bool>()){
                std::cout << t << "\n";
                break;
            }
        }
    }
}