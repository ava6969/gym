#pragma once
//
// Created by dewe on 6/5/22.
//

#include "common/wrapper.h"
#include "minigrid.h"

namespace gym{


    struct ViewSizeWrapper : ObservationWrapper2< MiniGridEnv, torch::Tensor   >{
        explicit ViewSizeWrapper( std::shared_ptr<MiniGridEnv> const& env, int agent_view_size=7 );
        torch::Tensor observation( std::unordered_map<std::string, cv::Mat> && x) const noexcept override;
    };

    struct ViewSizeWrapper2 : ObservationWrapper2< ObservationWrapper< MiniGridEnv>, cv::Mat >{
        explicit ViewSizeWrapper2( std::shared_ptr< ObservationWrapper< MiniGridEnv> > const& env, int agent_view_size=7);
        cv::Mat  observation( std::unordered_map<std::string, cv::Mat> && x) const noexcept override;
    };

    struct FlatObsWrapper : ObservationWrapper2< MiniGridEnv, std::vector<float>   >{
        explicit FlatObsWrapper( std::shared_ptr<MiniGridEnv> const& env);
        std::vector<float> observation( std::unordered_map<std::string, cv::Mat> &&) const noexcept override;
    private:
        std::vector<float> state;
    };

    struct RGBImgObsWrapper : ObservationWrapper< MiniGridEnv> {
        explicit RGBImgObsWrapper( std::unique_ptr<MiniGridEnv> env, int tile_size=8);
        std::unordered_map<std::string, cv::Mat> observation( std::unordered_map<std::string, cv::Mat> &&) const noexcept override;
    protected:
        int tile_size{};
    };

    struct RGBImgPartialObsWrapper : ObservationWrapper< MiniGridEnv> {
        explicit RGBImgPartialObsWrapper( std::unique_ptr<MiniGridEnv> env, int tile_size=8);
        std::unordered_map<std::string, cv::Mat> observation( std::unordered_map<std::string, cv::Mat> &&) const noexcept override;
    protected:
        int tile_size{};
    };
}