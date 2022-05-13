//
// Created by Ben Giacalone on 9/5/2021.
//
#include "common/utils.h"
#include "mountain_car.h"

namespace gym{

    gym::MountainCarEnv::MountainCarEnv() {
        // Define observation and action m_Space
        m_ObservationSpace = makeBoxSpace<float>(2);
        m_ActionSpace = makeDiscreteSpace(3);

        reset();
    }

    MountainCarEnv::StepT gym::MountainCarEnv::step(const ActionT &action)  noexcept{
        // Update velocity
        m_CarVel += (action - 1) * 0.001 + cos(3 * m_CarPos) * -0.0025;
        if (m_CarVel > MAX_VEL)
            m_CarVel = MAX_VEL;
        if (m_CarVel < -MAX_VEL)
            m_CarVel = -MAX_VEL;

        // Update car position
        m_CarPos += m_CarVel;
        if (m_CarPos > MAX_POS)
            m_CarPos = MAX_POS;
        if (m_CarPos < MIN_POS)
            m_CarPos = MIN_POS;

        if (m_CarPos == MIN_POS && m_CarVel < 0)
            m_CarVel = 0;

        return {{m_CarPos, m_CarVel} , -1.f, goal_achieved(), {}};
    }

    MountainCarEnv::ObservationT MountainCarEnv::reset()  noexcept {
        m_CarPos = -0.5;
        m_CarVel = 0.0;
        return {m_CarPos, m_CarVel};
    }

    void gym::MountainCarEnv::render(gym::RenderType) {
        // If a viewer does not exist, create it
        if (!m_Viewer) {
            m_Viewer = std::make_unique<Viewer>(400, 400, "MountainCar");
            m_Viewer->setBounds(0, 400, 0, 400);

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
        m_CarTransform.m_Translation = std::array<float, 2>{static_cast<float>(((m_CarPos - MIN_POS) / (MAX_POS - MIN_POS)) * 400),
                                                            static_cast<float>((sin(3 * m_CarPos) + 1) * 100 + 200)};

        m_Viewer->render();
    }
}

