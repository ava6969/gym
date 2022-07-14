//
// Created by dewe on 8/24/21.
//

#include <future>
#include <ATen/Tensor.h>
#include "lunarlandar.h"
#include "chrono"
#include "thread"
#include "spaces/util.h"

template<bool cont>
typename gym::LunarLandarEnv<cont>::ActionT heuristic(gym::LunarLandarEnv<cont>* env,
                             std::vector<float> const& s){
    auto angle_targ = (s.at(0) * 0.5) + (s.at(2) * 1.0);
    if (angle_targ > 0.4)
        angle_targ = 0.4;  // more than 0.4 radians (22 degrees) is bad
    if (angle_targ < -0.4)
        angle_targ = -0.4;

    auto hover_targ = 0.55 * std::abs(s.at(0));
    auto angle_todo = (angle_targ - s.at(4)) * 0.5 - (s.at(5)) * 1.0;
    auto hover_todo = (hover_targ - s.at(1)) * 0.5 -  (s.at(3)) * 0.5;

    if ( bool(s.at(6)) or bool(s.at(7)) )  // legs have contact
    {
        angle_todo = 0;
        hover_todo = (-(s.at(3)) * 0.5);
    }

    if constexpr( cont){
        return std::vector<float>{std::clamp<float>(hover_todo * 20 - 1, -1, 1),
                std::clamp<float>(-angle_todo * 20, -1, 1)};
    }else{
        int a = 0;
        if(hover_todo > std::fabs(angle_todo) and hover_todo > 0.05)
            a = 2;
        else if (angle_todo < -0.05)
            a = 3;
        else if (angle_todo > +0.05)
            a = 1;
        return a;
    }
}

template<bool cont>
void singlePlayer( gym::LunarLandarEnv<cont>& env, int seed=1){

    auto total_reward = 0.f;
    auto steps = 0;
    env.seed(seed);
    auto s = env.reset();
    std::vector<float> rews;
    while(true){

        auto a = heuristic(&env, s);
        auto resp = env.step(a);

        total_reward += resp.reward;
        rews.push_back(resp.reward);

        env.render();

        if(steps % 20 == 0 or resp.done){
            std::cout << "observation\t" << resp.observation << "\n"
            << "step\t" << steps << " reward\t" << total_reward << "\n";
        }

        steps++;

        if(resp.done)
        {
            break;
        }
        s = resp.observation;
    }
}

void singlePlayerCar( gym::CarRacing& env, int seed=1){

    auto total_reward = 0.f;
    auto steps = 0;
    env.seed(seed);

    for(int i =0; i < 5; i++){
        auto s = env.reset();
        std::vector<float> rews;
        while(true){

            auto a = env.actionSpace()->sample<std::vector<float>>();
            auto resp = env.step(a);

            total_reward += resp.reward;
            rews.push_back(resp.reward);

            env.render();

            steps++;

            if(resp.done)
            {
                break;
            }
            s = resp.observation;
        }
    }

}


template<class EnvT, bool cont=false>
void multiplayer(int players){

    std::vector<std::future<void>> ftrs(players);
    std::vector< EnvT > envs(players);
    int i = 1;
    std::transform(envs.begin(), envs.end(), ftrs.begin(), [i](auto& env) mutable {
        if constexpr( std::same_as<EnvT, gym::CarRacing>)
            return std::async(singlePlayerCar, std::ref(env), i++);
        else
            return std::async(singlePlayer<cont>, std::ref(env), i++);
    });

    for(auto& ftr: ftrs)
        ftr.wait();
}

int main(){
//    gym::LunarLandarEnv<false> discrete_env, cont_env;

//    std::cout << "Running Single Continuous Player\n";
//    singlePlayer(cont_env);
//
//    std::cout << "Running Single discrete Player\n";
//    singlePlayer(discrete_env);


//    std::cout << "Running Multiplayer Continuous Player\n";
//    multiplayer<true>(3);
//
//    std::cout << "Running Multiplayer Discrete Player\n";
//    multiplayer<false>(4);

    std::cout << "Running Multiplayer CarRacing Player\n";
    multiplayer<gym::CarRacing>(16);
}