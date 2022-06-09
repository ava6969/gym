//
// Created by dewe on 6/5/22.
//

#include "common/utils.h"
#include "wrappers.h"

namespace gym{

    ViewSizeWrapper::ViewSizeWrapper( std::shared_ptr<MiniGridEnv> const& env, int agent_view_size ):
    ObservationWrapper2< MiniGridEnv, torch::Tensor  >(
            makeBoxSpace<float>( 0, 255, {3, agent_view_size, agent_view_size}), env ){

        assert( agent_view_size % 2 == 1);
        assert( agent_view_size >= 3);
        m_Env->setAgentSize( agent_view_size );
    }

    torch::Tensor  ViewSizeWrapper::observation( std::unordered_map<std::string, cv::Mat> && _dict) const noexcept {
        auto image = _dict["image"];
        auto w = image.size[0], h = image.size[1], c = 3;
        torch::Tensor img = torch::from_blob(image.data, {w*h*c}, c10::kByte).view({w, h, 3}).permute({2, 0, 1});
        return img.to(c10::kFloat);
    }

    FlatObsWrapper::FlatObsWrapper(std::shared_ptr<MiniGridEnv> const& env):
            ObservationWrapper2< MiniGridEnv, std::vector<float> >(
                    flattenSpace<int>( env->observationSpace()->namedSpaces()["image"]->as<Box<uint8_t>>(), 0), env){
        state.resize( m_ObservationSpace->size()[0] );
    }

    std::vector<float> FlatObsWrapper::observation(std::unordered_map<std::string, cv::Mat> && _dict) const noexcept {
        cv::Mat image = _dict["image"];
        image.convertTo(image, CV_32F);
        memmove((void *) state.data(), image.data, sizeof(float)*state.size());
        return state;
    }

    RGBImgObsWrapper::RGBImgObsWrapper(std::unique_ptr<MiniGridEnv> env, int tile_size):
            ObservationWrapper< MiniGridEnv >( std::move(env) ){

        m_ObservationSpace->as<ADict>()->update("image",
                                                makeBoxSpace<uint8_t>( 0, 255, { m_Env->getWidth()*tile_size,
                                                                                 m_Env->getHeight()*tile_size, 3}) ) ;
        this->tile_size = tile_size;
        m_Env->setTileSize(tile_size);
    }

    unordered_map<std::string, cv::Mat>
            RGBImgObsWrapper::observation(unordered_map<std::string, cv::Mat> && x) const noexcept {
        x.insert_or_assign("image", m_Env->_render(false, tile_size));
        return x;
    }

    RGBImgPartialObsWrapper::RGBImgPartialObsWrapper(std::unique_ptr<MiniGridEnv> env, int tile_size):
    ObservationWrapper< MiniGridEnv> ( std::move(env) ) {
        auto size = m_Env->observationSpace()->namedSpaces().at("image")->size();
        m_ObservationSpace->as<ADict>()->update("image",
                                                makeBoxSpace<uint8_t>( 0, 255, { size[0]*tile_size,
                                                                                 size[1]*tile_size, 3}) ) ;
        this->tile_size = tile_size;
        m_Env->setTileSize(tile_size);
    }

    std::unordered_map<std::string, cv::Mat>
    RGBImgPartialObsWrapper::observation(unordered_map<std::string, cv::Mat> && x) const noexcept {
        x.insert_or_assign("image", m_Env->getObsRender( x.at("image"), tile_size));
        return x;
    }

    ViewSizeWrapper2::ViewSizeWrapper2( std::shared_ptr< ObservationWrapper< MiniGridEnv> > const& env,
                                        int agent_view_size):
            ObservationWrapper2< ObservationWrapper< MiniGridEnv>, cv::Mat  >(
                    env->observationSpace()->namedSpaces().at("image")->clone(), env ){
        auto casted =  env->try_cast<MiniGridEnv>();
        assert(casted);
        casted->setAgentSize( agent_view_size );
        m_ObservationSpace = makeBoxSpace<uint8_t>( 0, 255, { agent_view_size*casted->tileSize(),
                                                              agent_view_size*casted->tileSize(), 3})  ;

    }
    cv::Mat ViewSizeWrapper2::observation(unordered_map<std::string, cv::Mat> &&x) const noexcept {
        return x.at("image");
    }
}