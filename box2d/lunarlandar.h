//
// Created by dewe on 8/23/21.
//

#ifndef GYMENV_LUNARLANDAR_H
#define GYMENV_LUNARLANDAR_H

#include <map>
#include "env.h"
#include "utils.h"
#include "../common/rendering.h"

namespace gym{

    const static int FPS = 50;
    const static float SCALE = 30.0;  // affects how fast-paced the game is, forces should be adjusted as well
    const static float MAIN_ENGINE_POWER = 13.0;
    const static float SIDE_ENGINE_POWER = 0.6;
    const static float INITIAL_RANDOM = 1000.0; // Set 1500 to make game harder
    const static std::array<b2Vec2, 6> LANDER_POLY =
            {b2Vec2{-14, +17},
             {-17, 0},
             {-17, -10},
             {+17, -10},
             {+17, 0},
             {+14, +17}};
    const static int   LEG_AWAY = 20;
    const static int   LEG_DOWN = 18;
    const static int   LEG_W    = 2;
    const static int   LEG_H    = 8;
    const static int   LEG_SPRING_TORQUE = 40;
    const static float SIDE_ENGINE_HEIGHT = 14.0;
    const static float SIDE_ENGINE_AWAY = 12.0;
    const static int   VIEWPORT_W = 600;
    const static int   VIEWPORT_H = 400;
    const static float& W = VIEWPORT_W / SCALE;
    const static float& H = VIEWPORT_H / SCALE;
    int static  const& CHUNKS = 11;

    template<bool continuous>
    class LunarLandarEnv;

    struct UserData{
        b2Color m_Color1{}, m_Color2{};
        bool m_GroundContact{false};
        float ttl=0;
    };

    template<bool continuous>
    class ContactDetector : public b2ContactListener{

    private:
        LunarLandarEnv<continuous>* m_Env;

    public:
        explicit ContactDetector(LunarLandarEnv<continuous>* env):m_Env(env){}

        void setGroundContact(b2Contact* contact,
                              b2Body* leg,
                              bool value);

        void BeginContact(b2Contact* contact) final;

        void EndContact(b2Contact* contact) final;

    };

    template<bool continuous>
    class LunarLandarEnv : public Env<std::vector<float>,
            std::conditional_t<continuous, int, std::vector<float> > > {

    public:
        using ActionT =  std::conditional_t<continuous, int, std::vector<float> >;
        LunarLandarEnv();

        std::unique_ptr<class Viewer> m_Viewer;

        std::vector<float> reset()  noexcept final;

        StepResponse< std::vector<float> > step(ActionT const& action) noexcept final;

        void render(RenderType type) final;

        [[nodiscard]] inline
        b2Body* leg(int i) const { return m_Legs[i]; }

        [[nodiscard]] inline
        b2Body* lander() const{ return m_Lander; }

        inline void setGameOver(){ m_GameOver = true; }

        inline void resetGameOver(){ m_GameOver = false; }

        inline void seed(std::optional<uint64_t> const& seed) noexcept override{
            this->m_Device.seed(seed.value_or(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
        }

    private:
         std::unique_ptr<b2World> m_World;

         ContactDetector<continuous> contactDetector{this};

         b2Body* m_Moon{nullptr};
         b2Body* m_Lander{nullptr};
         std::vector<b2Body*> m_DrawList;
         std::vector<b2Body*> m_Legs{2, nullptr}, m_Particles;

         std::array<Vertices, CHUNKS-1> m_SkyPolys;

         UserData lunarLanderData, moonData;
         std::array<UserData, 2> legData;
         std::vector<UserData> particleData;

         float m_HelipadX1, m_HelipadX2, m_HelipadY;

         bool m_GameOver{false};

         std::optional<float> m_PrevShaping;

        void destroy()  noexcept;

        void cleanParticle(bool all)  noexcept;

        b2Body* createParticle(float mass, float x, float y, float tt1)  noexcept;
    };

    using LunarLandarEnvDiscrete = LunarLandarEnv<false>;
    using LunarLandarEnvContinuous = LunarLandarEnv<true>;
}

#endif //GYMENV_LUNARLANDAR_H
