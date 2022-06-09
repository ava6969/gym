//
// Created by dewe on 10/9/21.
//
#include "common/utils.h"
#include "opencv2/opencv.hpp"
#include "games.h"
#include "dm_lab.h"

namespace gym{

    std::map<std::string, std::string> DMLabEnv::makeConfig( DMLabEnv::Option& args){
        auto isTest = args.is_test;
        std::map<std::string, std::string> config;
        if(isTest){
            config["allowHoldOutLevels"] = "true";
            config["mixerSeed"] = "0x600D5EED";
        }
        config["logLevel"] = "WARN";

        auto dp = args.dataset_path;
        if(not dp.empty())
            config["datasetPath"] = dp;
        config["width"] = std::to_string(args.width);
        config["height"] = std::to_string(args.height);
        if(ALL_GAMES.contains(args.game))
            args.game = "contributed/dmlab30/" + args.game;

        return config;
    }

    DMLabEnv::DMLabEnv(Option  args):Env< cv::Mat , int>(),
            num_action_repeats(args.action_repeats),
            m_Lab(args.game,
                  {"RGB_INTERLEAVED"},
                  makeConfig(args),
                  args.renderer,
                  args.level_cache_dir ? new LevelCache(*args.level_cache_dir) : nullptr),
              m_ActionSet(args.action_set){

        Env<cv::Mat, int>::seed(args.seed);
        m_ObservationSpace = makeBoxSpace<>(0, 255, {args.height, args.width, 3});
        m_ActionSpace = makeDiscreteSpace(m_ActionSet.size());

    }

    cv::Mat DMLabEnv::reset() noexcept {
        m_Lab.reset();
        return observation();
    }

    StepResponse< cv::Mat > DMLabEnv::step(const ActionT &action) noexcept {
        auto reward = m_Lab.step(m_ActionSet[action], num_action_repeats);
        auto done = not m_Lab.isRunning();
        return { done ? cv::Mat{} : observation(),
                static_cast<float>(reward),
                done,
                {}};
    }

    void DMLabEnv::render( ) {

    }
}