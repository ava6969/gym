#pragma once
//
// Created by dewe on 6/29/22.
//
#include <iostream>
#include "chrono"
#include "vector"
#include "memory"
#include "boost/core/demangle.hpp"
#include "optional"


template<class EnvT, class PolicyT>
static inline auto speed_test_per_run(PolicyT policy, EnvT& env){

    auto start = std::chrono::high_resolution_clock::now();
    env.reset();
    auto duration = std::chrono::high_resolution_clock::now() - start;

    auto resetDuration = std::chrono::duration<double>(duration).count();

    auto done = false;

    start = std::chrono::high_resolution_clock::now();
    int episodeLength = 0;
    while (not done){
        auto stepResponse = env.step( policy(env) );
        done = stepResponse.done;
        episodeLength++;
    }
    duration = std::chrono::high_resolution_clock::now() - start;
    auto stepDuration = std::chrono::duration<double>(duration).count();

    return std::make_tuple(resetDuration, stepDuration, episodeLength);

}


template<class EnvT, class PolicyT, class ... Args>
static inline auto speed_test_per_run(PolicyT policy, Args ... args){
    return speed_test_per_run(policy, EnvT( std::forward<Args>(args) ... ) );
}

template<typename EnvT>
static inline int discreteRandomPolicy(EnvT const& env){
    return env.actionSpace()->template sample<int>();
}

template<typename EnvT>
static inline float continuousRandomPolicy(EnvT const& env){
    return env.actionSpace()->template sample<float>();
}

template<typename EnvT>
static inline std::vector<int> multiDiscreteRandomPolicy(EnvT const& env){
    return env.actionSpace()->template sample< std::vector<int> >();
}

template<typename EnvT>
static inline std::vector<float> multiContinuousRandomPolicy(EnvT const& env){
    return env.actionSpace()->template sample< std::vector<float> >();
}


template<typename T>
using DiscretePolicyT = int(*)(T const&);

template<typename T>
using ContinuousPolicyT = float(*)(T const&);

template<typename T>
using MultiDiscretePolicyT = std::vector<int>(*)(T const&);

template<typename T>
using MultiContinuousPolicyT = std::vector<float>(*)(T const&);


template<class EnvT, class PolicyT, class ... Args>
static inline auto speed_test(int runs,
                              PolicyT policy = discreteRandomPolicy<EnvT>,
                              std::optional<std::string> const& name=std::nullopt,
                              Args ... args){

    EnvT env ( std::forward<Args>(args) ...);
    for(int i =0; i < runs; i++){
        auto [resetResp, stepResponse, episodeLength] = speed_test_per_run<EnvT>(policy, env );

        std::cout << name.template value_or( boost::core::demangle( typeid(EnvT).name()) ) <<
                "\tRun " << i+1 << " --> AvgTime:\t Reset run time ==> " << resetResp <<
                  " s\t Avg step run time ==> " << stepResponse/episodeLength << " s\t" <<
                  "Episode Length: " << episodeLength << std::endl;
    }
}