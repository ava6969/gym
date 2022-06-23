//
// Created by dewe on 8/23/21.
//

#ifndef GYMENV_LUNARLANDAR_H
#define GYMENV_LUNARLANDAR_H

#include <map>
#include "env.h"
#include "utils.h"
#include "../common/rendering.h"
#include "contact_listen.h"


namespace gym{

    template<bool continuous>
    class LunarLandarEnv;

    struct Leg : box2d::DrawableBodyBase{
        bool groundContact{false};
    };

    template<bool continuous>
    class LunarLandarEnv : public
            Env<std::vector<float>, std::conditional_t<continuous, std::vector<float>, int > > {

    public:
        using ActionT = std::conditional_t<continuous, std::vector<float>, int >;
        LunarLandarEnv();

        std::unique_ptr<class Viewer> m_Viewer;

        std::vector<float> reset()  noexcept final;

        StepResponse< std::vector<float> > step( ActionT const& action) noexcept final;

        void render() final;

        [[nodiscard]] inline
        Leg& leg(int i) { return m_Legs[i]; }

        [[nodiscard]] inline
        b2Body* lander() const{ return m_Lander.body; }

        inline void setGameOver(){ m_GameOver = true; }

        inline void resetGameOver(){ m_GameOver = false; }

        inline void seed(std::optional<uint64_t> const& seed) noexcept override{
            this->m_Device.seed(seed.value_or(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
        }

    private:

        constexpr  static int FPS = 50;
        constexpr  static float SCALE = 30.0;  // affects how fast-paced the game is, forces should be adjusted as well
        constexpr  static float MAIN_ENGINE_POWER = 13.0;
        constexpr  static float SIDE_ENGINE_POWER = 0.6;
        constexpr  static float INITIAL_RANDOM = 1000.0; // Set 1500 to make game harder
        inline const static std::array<b2Vec2, 6> LANDER_POLY =
                {b2Vec2{-14, +17},
                 {-17, 0},
                 {-17, -10},
                 {+17, -10},
                 {+17, 0},
                 {+14, +17}};
        constexpr  static int   LEG_AWAY = 20;
        constexpr  static int   LEG_DOWN = 18;
        constexpr  static int   LEG_W    = 2;
        constexpr  static int   LEG_H    = 8;
        constexpr  static int   LEG_SPRING_TORQUE = 40;
        constexpr  static float SIDE_ENGINE_HEIGHT = 14.0;
        constexpr  static float SIDE_ENGINE_AWAY = 12.0;
        constexpr  static int   VIEWPORT_W = 600;
        constexpr  static int   VIEWPORT_H = 400;
        constexpr  static float W = VIEWPORT_W / SCALE;
        constexpr  static float H = VIEWPORT_H / SCALE;
        constexpr  int static CHUNKS = 11;

         std::unique_ptr<b2World> m_World;

         ContactDetector<continuous> contactDetector;

        box2d::DrawableBodyBase m_Moon;
         box2d::DrawableBodyBase m_Lander;
         std::vector<box2d::DrawableBodyBase> m_DrawList;
         std::array<Leg, 2> m_Legs;
         std::vector<box2d::Particle> m_Particles;

         std::array<Vertices, CHUNKS-1> m_SkyPolys;
         float m_HelipadX1, m_HelipadX2, m_HelipadY;

         bool m_GameOver{false};

         std::optional<float> m_PrevShaping;

        void destroy()  noexcept;

        void cleanParticle(bool all)  noexcept;

        b2Body* createParticle(float mass, float x, float y, float tt1)  noexcept;
    };

    extern template class LunarLandarEnv<true>;
    extern template class LunarLandarEnv<false>;
    using LunarLandarEnvDiscrete = LunarLandarEnv<false>;
    using LunarLandarEnvContinuous = LunarLandarEnv<true>;
}

#endif //GYMENV_LUNARLANDAR_H
