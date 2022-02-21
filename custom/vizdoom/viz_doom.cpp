//
// Created by dewe on 10/9/21.
//

#include "opencv2/opencv.hpp"
#include "viz_doom.h"

namespace gym{

    VizDoomEnv::VizDoomEnv(OptionalArgMap const& arg): TensorEnv<VizDoomEnv>(arg){
        m_Health = getArg("health", false);
        m_Position = getArg("position", false);
        m_Labels = getArg("labels", false);
        m_Depth = getArg("depth", false);
        auto level = getArg("level"s, 7);

        auto config = CONFIGS[level];
        std::filesystem::path scenarios_dir = std::any_cast<std::string>(arg->at("scenario_path"s));

        scenarios_dir /= config.first;
        m_Game.loadConfig(scenarios_dir.string());
        m_Game.setScreenResolution(vizdoom::RES_640X480);
        m_Game.setScreenFormat(vizdoom::RGB24);

        m_Game.setWindowVisible(false);
        m_Game.setDepthBufferEnabled(m_Depth);
        m_Game.setLabelsBufferEnabled(m_Labels);
        m_Game.clearAvailableGameVariables();

        if(m_Position){
            m_Game.addAvailableGameVariable(vizdoom::GameVariable::POSITION_X);
            m_Game.addAvailableGameVariable(vizdoom::GameVariable::POSITION_Y);
            m_Game.addAvailableGameVariable(vizdoom::GameVariable::POSITION_Z);
            m_Game.addAvailableGameVariable(vizdoom::GameVariable::ANGLE);
        }

        if(m_Health){
            m_Game.addAvailableGameVariable(vizdoom::HEALTH);
        }

        m_Game.init();

        m_ActionSpace = makeDiscreteSpace(config.second);

        auto[h, w, c] = std::make_tuple(m_Game.getScreenHeight(),
                                        m_Game.getScreenWidth(),
                                        m_Game.getScreenChannels());
        m_Resolution = {h, w, c};

        auto ptr = m_Args->find("screen_size");
        if(ptr != m_Args->end()){
            auto sz = std::any_cast<long>(ptr->second);
            m_NewSize = sz;
            h = sz;
            c = sz;
        }

        NamedSpaces spaces;
        spaces["screen"] =  makeBoxSpace<uint8_t>(0, 255, {h, w, c});
        lastGoodState["screen"] = torch::zeros({c, h, w});
        if(m_Depth){
            spaces["depth"] = makeBoxSpace<uint8_t>(0, 255, {h, w});
            lastGoodState["depth"] = torch::zeros({h, w});
        }

        if(m_Labels){
            spaces["labels"] = makeBoxSpace<uint8_t>(0, 255, {h, w});
            lastGoodState["labels"] = torch::zeros({h, w});
        }

        if(m_Position){
            spaces["position"] = makeBoxSpace<float>(4);
            lastGoodState["position"] = torch::zeros({4});
        }

        if(m_Health){
            spaces["health"] = makeBoxSpace<float>(1);
            lastGoodState["health"] = torch::zeros({1});
        }

        if(spaces.size() == 1){
            m_ObservationSpace = std::move(spaces["screen"]);
        }else{
            m_ObservationSpace = makeDictionarySpace(std::move(spaces));
        }

        ZERO.resize(config.second);

    }

    void VizDoomEnv::render(RenderType ){
        m_State = m_Game.getState();
        if(m_State->screenBuffer){
            auto img = parseScreen();
            auto cv_img = cv::Mat(img.size(1), img.size(2), CV_8UC3, img.permute({2, 0, 1}).data_ptr<uint8_t>());
            cv::cvtColor(cv_img, cv_img, cv::COLOR_BGR2RGB);
            cv::imshow("DOOM", cv_img);
        }
    }

    TensorDict VizDoomEnv::reset() noexcept{
        m_Game.newEpisode();
        m_State = m_Game.getState();
        return collectObservation();
    }

    StepResponse<TensorDict> VizDoomEnv::step(const torch::Tensor &action) noexcept{
        auto act = ZERO;
        act[action.item<long>()] = 1;

        auto reward = m_Game.makeAction(act);
        m_State = m_Game.getState();
        auto done = m_Game.isEpisodeFinished();
        return {collectObservation(), static_cast<float>(reward), done, {}};
    }

    TensorDict VizDoomEnv::collectObservation() noexcept{

        if(m_State){
            lastGoodState["image"] = parseScreen();
            if(m_Depth){
                auto[h, w, _] = m_Resolution;
                lastGoodState["depth"] = torch::tensor(*m_State->depthBuffer).view({h, w});
            }
            if(m_Labels){
                auto[h, w, _] = m_Resolution;
                lastGoodState["depth"] = torch::tensor(*m_State->labelsBuffer).view({h, w});
            }
            if(m_Position){
                lastGoodState["position"] = torch::from_blob(m_State->gameVariables.data(), {4},
                                                             torch::kFloat64).toType(torch::kFloat32);
                if(m_Health){
                    lastGoodState["health"] = torch::tensor({m_State->gameVariables[4]});
                }
            }else if(m_Health){
                lastGoodState["health"] = torch::tensor({m_State->gameVariables[0]});
            }

        }

        return lastGoodState;
    }

    torch::Tensor VizDoomEnv::resize3D(vector<uint8_t> &x) noexcept{
        auto [h, c, _] = m_Resolution;
        cv::Mat mat(h, c, CV_8UC3, x.data());
        auto n = m_NewSize.value();
        cv::resize(mat, mat, {n, n}, 0, 0 , cv::INTER_AREA);
        return torch::from_blob(mat.data, {n*n*3}, torch::kUInt8).view({n, n, 3}).permute({2, 0, 1});
    }

}