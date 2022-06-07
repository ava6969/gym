//
// Created by dewe on 6/4/22.
//

#include "minigrid.h"
#include "rendering.h"
#include "object.h"

namespace mg{


    WorldObj::WorldObj(Object type, Color color):
            type(type),
            color(color) {

    }

    void WorldObj::resetPosition(Point pos){
        init_pos = pos;
        cur_pos = pos;
    }

    WorldObj::Ptr WorldObj::decode(State const& state){
        auto obj_type = toEnum<Object>(state.type);
        auto color = toEnum<Color>(state.color);

        if( obj_type == Object::Empty or obj_type == Object::Unseen)
            return nullptr;

        auto is_open = (state.state == 0);
        auto is_locked = (state.state == 2);

        switch (obj_type) {
            case Object::Wall:
                return std::make_shared< Wall >(color);
            case Object::Floor:
                return std::make_shared< Floor >(color);
            case Object::Door:
                return std::make_shared< Door >(color, is_open, is_locked);
            case Object::Key:
                return std::make_shared< Key >(color);
            case Object::Ball:
                return std::make_shared< Ball >(color);
            case Object::Box:
                return std::make_shared< Box >(color);
            case Object::Goal:
                return std::make_shared< Goal >();
            case Object::Lava:
                return std::make_shared< Lava >();
            default:
                throw std::runtime_error("Un-decodable Object Type");
        }
    }

    void Goal::render(cv::Mat& img) {
        fill_coords(img, point_in_rect( {0, 1}, {0, 1}), ColorMap[color]);
    }

    void Floor::render(cv::Mat& img) {
        fill_coords(img, point_in_rect( {0.031, 1}, {0.031, 1}), ColorMap[color] / 2);
    }

    void Lava::render(cv::Mat& img) {
        fill_coords(img, point_in_rect( {0, 1}, {0, 1}), {255, 128, 0});
        auto black = std::array<uint8_t, 3>{0, 0, 0};
        for( auto i : {0.f, 1.f, 2.f}){
            auto ylo = 0.3f + 0.2f * i;
            auto yhi = 0.4f + 0.2f * i;
            fill_coords(img, point_in_line({0.1, ylo}, {0.3, yhi}, 0.03), black);
            fill_coords(img, point_in_line({0.3, yhi}, {0.5, ylo}, 0.03), black);
            fill_coords(img, point_in_line({0.5, ylo}, {0.7, yhi}, 0.03), black);
            fill_coords(img, point_in_line({0.7, yhi}, {0.9, ylo}, 0.03), black);
        }
    }

    void Wall::render(cv::Mat& img) {
        fill_coords(img, point_in_rect( {0, 1}, {0, 1}), ColorMap[color]);
    }

    bool Door::toggle(gym::MiniGridEnv& env, Point pos) {

        if( is_locked ){
            if( dynamic_pointer_cast<Key>(env.carrying()) and env.carrying()->getColor() == color ){
                is_locked = false;
                is_open = true;
                return true;
            }
            return false;
        }
        is_open = not is_open;
        return true;

    }

    WorldObj::State Door::encode() const {
        uint8_t state = is_open ? 0 : is_locked ? 2 : 1;
        return {toIndex(type), toIndex(color), state};
    }

    void Door::render(cv::Mat& img) {
        auto c = ColorMap[color];
        auto black = std::array<uint8_t, 3>{0, 0, 0};

        if( is_open ){
            fill_coords(img, point_in_rect( {0.88, 1}, {0, 1}), c);
            fill_coords(img, point_in_rect({0.92, 0.96}, {0.04, 0.96}), black);
            return;
        }

        if(is_locked){
            fill_coords(img, point_in_rect( {0, 1}, {0, 1}), c);
            fill_coords(img, point_in_rect({0.06, 0.94}, {0.06, 0.94}), c * 0.45f );
            fill_coords(img, point_in_rect({0.52, 0.75}, {0.50, 0.56}), c);
        }else{
            fill_coords(img, point_in_rect( {0, 1}, {0, 1}), c);
            fill_coords(img, point_in_rect({0.04, 0.96}, {0.04, 0.96}), black);
            fill_coords(img, point_in_rect({0.08, 0.92}, {0.08, 0.92}), c);
            fill_coords(img, point_in_rect({0.12, 0.88}, {0.12, 0.88}), black);
            fill_coords(img, point_in_circle(0.75f, 0.50f, 0.08), c);
        }
    }

    void Key::render(cv::Mat& img) {
        auto c = ColorMap[color];
        auto black = std::array<uint8_t, 3>{0, 0, 0};

        fill_coords(img, point_in_rect({0.5, 0.63}, {0.31, 0.88}), c);

        fill_coords(img, point_in_rect({0.38, 0.50}, {0.59, 0.66}), c);
        fill_coords(img, point_in_rect({0.38, 0.50}, {0.81, 0.88}), c);

        fill_coords(img, point_in_circle(0.56f, 0.28, 0.190), c);
        fill_coords(img, point_in_circle(0.56, 0.28, 0.064), black);

    }

    void Ball::render(cv::Mat& img) {
        fill_coords(img, point_in_circle( 0.5, 0.5, 0.31 ), ColorMap[color]);
    }

    void Box::render(cv::Mat& img) {
        auto c = ColorMap[color];
        auto black = std::array<uint8_t, 3>{0, 0, 0};

        fill_coords(img, point_in_rect({0.12, 0.88}, {0.120, 0.88}), c);
        fill_coords(img, point_in_rect({0.18, 0.82}, {0.18, 0.82}), black);
        fill_coords(img, point_in_rect({0.16, 0.84}, {0.47, 0.53}), c);

    }

    bool Box::toggle(gym::MiniGridEnv& env, Point pos)  {
        env.getGrid().set(pos, contains);
        return true;
    }

}