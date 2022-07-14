#pragma once
//
// Created by dewe on 6/16/22.
//


#include "env.h"

#include "contact_listen.h"
#include "opencv2/opencv.hpp"
#include "car_dynamics.h"

namespace gym {

class CarRacing : public Env<cv::Mat, std::vector<float>>{

public:
        explicit CarRacing(bool hide=false, int verbose=1);
        CarRacing(CarRacing &&)=default;

        ObservationT reset() noexcept override;

        StepT step(const ActionT &action) noexcept override;
        void step() noexcept;

        void render() override;
        inline auto win() { return viewer->win(); }
        void initRenderer();
        ~CarRacing() override;

        friend class FrictionDetector;

public:

    static constexpr int STATE_W = 96 ,  // less than Atari 160x192
    STATE_H = 96,
            VIDEO_W = 600,
            VIDEO_H = 400,
            WINDOW_W = 1000,
            WINDOW_H = 800;

    static constexpr float SCALE       = 6.0,        // Track scale
    TRACK_RAD   = 900/SCALE,  // Track is heavily morphed circle with this radius
    PLAYFIELD   = 2000/SCALE, // Game over boundary
    FPS         = 50,         // Frames per second
    ZOOM        = 2.7;        // Camera zoom
    static constexpr bool ZOOM_FOLLOW = true;       // Set to False for fixed view (don't use zoom)

    static constexpr float TRACK_DETAIL_STEP = 21/SCALE,
            TRACK_TURN_RATE = 0.31,
            TRACK_WIDTH = 40/SCALE,
            BORDER = 8/SCALE;

    static constexpr int BORDER_MIN_COUNT = 4;

    inline static const  b2Color ROAD_COLOR = {0.4, 0.4, 0.4};
    enum class RenderMode{
        StatePixel, Human, RGBArray
    };
    private:

        bool hide_window{false};

        float reward{}, prev_reward{}, start_alpha{};
        std::vector< std::vector<double> > track;
        int tileVisitedCount{};
        int verbose{};
        std::unique_ptr<class FrictionDetector> contactListener_keepref;
        box2d::util::PolygonFixtureDef<4> fd_tile;
        std::unique_ptr<Viewer> viewer{nullptr};
        Transform transform;

        std::vector< std::pair< std::array<b2Vec2, 4>, std::array<double, 3> > > road_poly;
        float t= 0;
        cv::Mat state;
        std::unique_ptr<class FTLabel> score_label;
        std::shared_ptr<class GLFont> _font;

        std::vector<box2d::Tile> road;
        std::optional<box2d::Car> car;

        std::unique_ptr<b2World> world;

        bool createTrack();

        void renderRoad();

        void renderIndicators(float W, float H);

        float norm(std::array<float, 2> const& x);

        template<RenderMode m> cv::Mat _render();

        void destroy();

    };

    extern template cv::Mat CarRacing::_render<CarRacing::RenderMode::Human>();
    extern template cv::Mat CarRacing::_render<CarRacing::RenderMode::RGBArray>();
    extern template cv::Mat CarRacing::_render<CarRacing::RenderMode::StatePixel>();

}