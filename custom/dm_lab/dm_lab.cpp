//
// Created by dewe on 10/9/21.
//
#include "common/utils.h"
#include "opencv2/opencv.hpp"
#include "games.h"
#include "dm_lab.h"

#include <utility>

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

    DMLabEnv2::DMLabEnv2(std::shared_ptr<DMLabEnv> env) :
            ObservationWrapper2< DMLabEnv, std::unordered_map<std::string, torch::Tensor>>(nullptr, std::move(env)){

        NamedSpaces spaces;
        spaces["image"] = m_Env->observationSpace()->clone();
        spaces["reward"] = makeBoxSpace<float>(1);
        spaces["action"] = makeBoxSpace<float>(m_Env->N());
        N = m_Env->N();
        lastAction = 0;
        lastReward = 0;
    }

    std::unordered_map<std::string, torch::Tensor> DMLabEnv2::observation(cv::Mat && x) const noexcept {
        auto[h,w] = std::tie(x.size[0], x.size[1]);
        int c = 3;
        return {{"image", torch::from_blob(x.data, {h*w*c}, c10::kByte).view({h, w, c}).permute({2, 0, 1}) / 255 },
                {"reward", torch::tensor(lastReward)},
                {"action", torch::one_hot( torch::tensor(lastAction),  N).to(c10::kFloat) }};
    }
}