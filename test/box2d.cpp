//
// Created by dewe on 8/24/21.
//

#include <future>
#include "lunarlandar.h"
#include "chrono"
#include "thread"
#include "spaces/util.h"

std::vector<float> heuristic(gym::LunarLandarEnv* env,
                             torch::Tensor const& k){
    std::vector<float> s(k.size(0));
    memcpy(s.data(), k.data_ptr<float>(), sizeof(float) * s.size());
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

    std::vector<float> a = {0.f};

    if(env->continuous()){
        return {std::clamp<float>(hover_todo * 20 - 1, -1, 1),
                std::clamp<float>(-angle_todo * 20, -1, 1)};
    }else{
        a = {0};
        if(hover_todo > std::fabs(angle_todo) and hover_todo > 0.05)
            a = {2};
        else if (angle_todo < -0.05)
            a = {3};
        else if (angle_todo > +0.05)
            a = {1};
    }
    return a;
}

void singlePlayer( gym::LunarLandarEnv& env){

    auto total_reward = 0.f;
    auto steps = 0;
    env.seed(1);
    torch::Tensor s = env.reset().at("observation").clone();
    std::vector<float> rews;
    while(true){

        auto a = heuristic(&env, s);

        auto resp = env.step(torch::tensor(a));

        total_reward += resp.reward;
        rews.push_back(resp.reward);

//        env.render(gym::RenderType::HUMAN);

        if(steps % 20 == 0 or resp.done){
            std::cout << "observation\t" << resp.m_NextObservation.at("observation").unsqueeze(0) << "\n"
            << "step\t" << steps << " reward\t" << total_reward << "\n";
        }

        steps++;

        if(resp.done)
        {
            break;
        }
        s = resp.m_NextObservation.at("observation").clone();
    }
}

void multiplayer(int players){

    std::vector<std::future<void>> ftrs(players);
    std::vector< gym::LunarLandarEnv > envs(players);
    std::transform(envs.begin(), envs.end(), ftrs.begin(), [](auto& env){
        return std::async(singlePlayer, std::ref(env));
    });

    for(auto& ftr: ftrs)
        ftr.wait();
}

int main(){
    gym::LunarLandarEnv env;

    singlePlayer(env);
    std::cout << "\n";
    singlePlayer(env);
    std::cout << "\n";
    singlePlayer(env);
    std::cout << "\n";
    singlePlayer(env);
}