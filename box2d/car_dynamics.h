#pragma once
#include "utils.h"


//
// Created by dewe on 6/15/22.
//

constexpr float SIZE = 0.02f;
constexpr double ENGINE_POWER = 100000000*SIZE*SIZE,
WHEEL_MOMENT_OF_INERTIA = 4000*SIZE*SIZE,
FRICTION_LIMIT          = 1000000*SIZE*SIZE;     // friction ~= mass ~= size^2 (calculated implicitly using density)
constexpr int WHEEL_R  = 27, WHEEL_W  = 14;

constexpr float WHEELPOS[][2] = {{-55,+80}, {+55,+80}, {-55,-82}, {+55,-82} };
constexpr float HULL_POLY1[][2] = { {-60,+130}, {+60,+130}, {+60,+110}, {-60,+110}};
constexpr float HULL_POLY2[][2] = {{-15,+120}, {+15,+120},{+20, +20}, {-20,  20}};

constexpr float HULL_POLY3[][2] = {
        {+25, +20},
        {+50, -10},
        {+50, -40},
        {+20, -90},
        {-20, -90},
        {-50, -40},
        {-50, -10},
        {-25, +20}};

constexpr float HULL_POLY4[][2] = {{-50, -120}, {+50, -120}, {+50, -90}, {-50, -90}};

const b2Color WHEEL_COLOR = {0.0, 0.0, 0.0},
WHEEL_WHITE = {0.3,0.3,0.3},
MUD_COLOR   = {0.4,0.4,0.0};

namespace gym::box2d {


    struct TileBase : DrawableBodyBase{
        float road_friction{};
        bool road_visited{false};

        explicit TileBase(b2Body* body): DrawableBodyBase(body){}

        void setUserData() override{
            body->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);
        }
    };
    using Tile = std::unique_ptr<TileBase>;

    struct WheelBase : DrawableBodyBase{
        float gas{}, brake{}, steer{}, phase{}, omega{}, wheel_rad{};

        std::optional<b2Vec2> skid_start{};
        std::optional<Particle> skid_particle{};
        b2RevoluteJoint* joint{nullptr};
        std::unordered_map<std::uintptr_t , TileBase*> tiles{};

        explicit WheelBase(b2Body* body): DrawableBodyBase(body){}
        void setUserData() override{
            body->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);
        }
        inline void add(TileBase* t){
            auto i = reinterpret_cast<uintptr_t>(t);
            tiles[i] = t;
        }

        inline void remove(TileBase* t){
            auto i = reinterpret_cast<uintptr_t>(t);
            tiles.erase(i);
        }
    };
    using Wheel = std::unique_ptr<WheelBase>;

    class Car {

    public:

        Car()=default;
        Car(b2World* world, float init_angle, float init_x, float init_y);
        void gas(float gas);
        void brake(float b);
        void steer(float s);
        void step(float dt);

        template<bool draw_particles=true>
        void draw(class Viewer&);

        Particle& createParticle(b2Vec2 const& point1, b2Vec2 const&  point2, bool grass);

        inline void fuelSpent(float f) { fuel_spent = f; }
        [[nodiscard]] inline std::array<float, 2> hullPosition() const { auto p = m_Hull->body->GetPosition(); return {p.x, p.y}; }
        [[nodiscard]] inline std::array<float, 2> hullLinearVelocity() const { auto v = m_Hull->body->GetLinearVelocity(); return {v.x, v.y}; }
        [[nodiscard]] inline float hullAngularVelocity() const { return m_Hull->body->GetAngularVelocity();  }
        [[nodiscard]] inline float hullAngle() const { return m_Hull->body->GetAngle();  }
        [[nodiscard]] inline auto* wheels(int idx) const { return m_Wheels.at(idx).get(); }
        void destroy();

    private:
        b2World* m_World{};
        DrawableBody m_Hull;
        float fuel_spent{};

        std::vector<Wheel> m_Wheels;
        std::deque<Particle> particles;
        std::vector<DrawableBodyBase*> m_drawList;

        Color colorAttr;

    };

    template<>
    void Car::draw<true>(Viewer& viewer);

    template<>
    void Car::draw<false>(Viewer& viewer);

} // gym