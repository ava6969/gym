//
// Created by Ben Giacalone on 9/5/2021.
//
#include "common/utils.h"
#include "../common/rendering.h"
#include "mountain_car.h"

namespace gym{

    gym::MountainCarEnv::MountainCarEnv(int goal_velocity):goal_velocity(goal_velocity) {
        // Define observation and action m_Space
        m_ObservationSpace = makeBoxSpace<float>({MIN_POS, -MAX_VEL}, {MAX_POS, MAX_VEL}, 2);
        m_ActionSpace = makeDiscreteSpace(3);
        MountainCarEnv::seed(std::nullopt);

    }

    MountainCarEnv::StepT gym::MountainCarEnv::step(const ActionT &action)  noexcept{
        // Update velocity
        auto[position, velocity] = std::tie(m_State[0], m_State[1]);
        velocity += float(action - 1) * FORCE + cos(3 * position) * -GRAVITY;
        velocity = std::clamp<double>(velocity, -MAX_VEL, MAX_VEL);

        // Update car position
        position += velocity;
        position = std::clamp<double>(position, MIN_POS, MAX_POS);

        if (position == MIN_POS && velocity < 0)
            velocity = 0;

        auto done = bool(position >= GOAL_POS and velocity >= goal_velocity);
        constexpr auto reward = -1.0;

        m_State = {position, velocity};
        return {m_State , reward, done, {}};
    }

    MountainCarEnv::ObservationT MountainCarEnv::reset()  noexcept {
        m_State = {_np_random.uniform(-0.6, -0.4), 0};
        return m_State;
    }

    void gym::MountainCarEnv::render(){

        constexpr float screen_width = 600, screen_height = 400,
        world_width=MAX_POS-MIN_POS,
        scale = screen_width/world_width,
        carwidth=40,
        carheight=20;

        // If a viewer does not exist, create it
        if (!m_Viewer) {
            m_Viewer = std::make_unique<Viewer>(screen_width, screen_height, "MountainCar");
//            m_Viewer->setBounds(0, 400, 0, 400);


            // Create rendering geometry
            m_CarColor = Color {0.0, 0.0, 0.0};
            m_CarTransform = Transform {};
            int res = 30;
            float radius = 10;
            for(int i = 0; i < res; i++){
                auto ang = 2.f * M_PI * i / res;
                m_CarGeom.m_Vertex.emplace_back(std::array<float, 2>{static_cast<float>(cos(ang) * radius),
                                                                     static_cast<float>(sin(ang) * radius)});
            }
            m_CarGeom.addAttr(&m_CarColor);
            m_CarGeom.addAttr(&m_CarTransform);
            m_Viewer->addGeom(&m_CarGeom);

            m_TrackColor = Color {0.0, 0.0, 0.0};
            int segments = 100;
            for (int i = 0; i < segments; i++) {
                auto x = MIN_POS + (MAX_POS - MIN_POS) * i / segments;
                auto y = sin(3 * x);
                auto vX = ((x - MIN_POS) / (MAX_POS - MIN_POS)) * 400;
                auto vY = (y + 1) * 100 + 200;
                m_TrackGeom.m_Vertex.emplace_back(std::array<float, 2>{vX, vY});
            }
            m_CarGeom.addAttr(&m_TrackColor);
            m_Viewer->addGeom(&m_TrackGeom);
        }

        // Render car
        auto pos = m_State[0];
        m_CarTransform.m_Translation = std::array<float, 2>{static_cast<float>(((pos - MIN_POS) / (MAX_POS - MIN_POS)) * 400),
                                                            static_cast<float>((sin(3 * pos) + 1) * 100 + 200)};

        m_Viewer->render();
    }
}

