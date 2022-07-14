//
// Created by dewe on 8/22/21.
//

#ifndef GYMENV_CARTPOLE_H
#define GYMENV_CARTPOLE_H

#include "env.h"
#include "common/rendering.h"

namespace gym{
    class CartPoleEnv : public Env<std::vector<double>, int> {

    public:

        explicit CartPoleEnv();

        void render() final;

        std::vector<double> reset() noexcept final;

        StepT step(ActionT const& action) noexcept final;

    private:
        std::unique_ptr<Viewer> m_Viewer;
        const float m_Gravity{9.8},
        m_MassCart{1.0},
        m_MassPole{0.1},
        m_TotalMass{m_MassPole + m_MassCart},
        m_Length{0.5},  // actually half the pole's length
        m_PoleMassLength{m_MassPole * m_Length},
        m_ForceMag{10.0},
        m_Tau{0.02},  // seconds between state updates
        m_ThetaThresholdRadians{12 * 2 * M_PI / 360}, // Angle at which to fail the episode
        m_XThreshold{2.4};

        std::string m_KinematicsIntegrator{"euler"};

         std::vector<double> m_State{};

         std::optional<int> m_StepsBeyondDone;

        Transform m_CartTransform, m_PoleTransform;
        FilledPolygon m_Pole, m_Cart, m_Axle;
        Line m_Track;
    };
}

#endif //GYMENV_CARTPOLE_H
