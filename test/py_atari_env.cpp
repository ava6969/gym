//
// Created by dewe on 8/31/21.
//

#include <python_gym/python_env.h>
#include "catch.hpp"
#include "atari/atari_env.h"
#include "wrappers/vec_env/vec_atari.h"
#include "wrappers/vec_env/sync_env.h"

constexpr bool dict_observation_type = false;
constexpr bool continuous_action_type = false;
constexpr bool atari_env_type = true;

using EnvT = gym::PythonEnv<dict_observation_type, continuous_action_type, atari_env_type>;
using SyncT = gym::SyncVecEnv<EnvT>;

TEST_CASE("Py Air Raid"){

    auto env = std::make_unique< EnvT >("PongNoFrameskip-v4");

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

    std::vector< std::unique_ptr< EnvT > > envs;
    envs.emplace_back( std::make_unique< EnvT >( "AirRaid-v0" ) );

    auto sync_env = SyncT::make( std::move(envs), true );

    std::unique_ptr< gym::VecEnv<dict_observation_type> > atariProcessing =
            std::make_unique< gym::VecNormAndPermute<dict_observation_type> >(
                    std::move(sync_env) );

    auto space = atariProcessing->observationSpace();
    for(int i =0; i < 5; i++){

        std::cout << atariProcessing->reset() << "\n";
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