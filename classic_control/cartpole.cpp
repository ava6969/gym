//
// Created by dewe on 8/22/21.
//
#include "common/utils.h"
#include "cartpole.h"

namespace gym{

    CartPoleEnv::CartPoleEnv(Kwargs const&){
        auto&& high = {m_XThreshold * 2,
                       std::numeric_limits<float>::max(),
                       m_ThetaThresholdRadians * 2,
                       std::numeric_limits<float>::max()};

        m_ActionSpace = makeDiscreteSpace(2);
        m_ObservationSpace = makeBoxSpace<float>(-high, high, 4);
    }

    CartPoleEnv::ObservationT CartPoleEnv::reset()  noexcept{

       std::uniform_real_distribution<float> dist(-0.05f, 0.05f);

       m_State = {dist(m_Device) ,
                  dist(m_Device) ,
                  dist(m_Device) ,
                  dist(m_Device)};

       m_StepsBeyondDone = std::nullopt;
        timeStep = 0;
        return m_State;
    }

    CartPoleEnv::StepT CartPoleEnv::step( CartPoleEnv::ActionT const& action)  noexcept{

        auto[x, xDot, theta, thetaDot] = std::make_tuple(m_State[0], m_State[1],m_State[2],m_State[3]);

        auto force = (action == 1) ? m_ForceMag : -m_ForceMag;

        auto cosTheta = float( std::cos(theta) );
        auto sinTheta = float( std::sin(theta) );

        auto temp = float(force + m_PoleMassLength * (thetaDot * thetaDot) * sinTheta )
                / m_TotalMass;

        float thetaAcc = (m_Gravity * sinTheta - cosTheta * temp) /
                float(m_Length * (4.0 / 3.0 - m_MassPole * (cosTheta * cosTheta) / m_TotalMass));

        float xAcc = temp - m_PoleMassLength * thetaAcc * cosTheta /  m_TotalMass;

        if (m_KinematicsIntegrator == "euler"){
            x = x + m_Tau * xDot;
            xDot = xDot + m_Tau * xAcc;
            theta = theta + m_Tau * thetaDot;
            thetaDot = thetaDot + m_Tau * thetaAcc;
        }else{
            xDot = xDot + m_Tau * xAcc;
            x = x + m_Tau * xDot;
            thetaDot = thetaDot + m_Tau * thetaAcc;
            theta = theta + m_Tau * thetaDot;
        }

        m_State = {x, xDot, theta, thetaDot};

        auto done = bool( timeStep == 500
                or x < -m_XThreshold
                or x > m_XThreshold
                or theta < -m_ThetaThresholdRadians
                or theta > m_ThetaThresholdRadians);

        float reward;
        if (not done){
            reward = 1.0;
        }else if(not m_StepsBeyondDone){
            m_StepsBeyondDone = 0;
            reward = 1.0;
        }else{
            if (m_StepsBeyondDone == 0){
                std::cerr << "You are calling 'step()' even though this "
                             "environment has already returned done = True. You "
                             "should always call 'reset()' once you receive 'done = "
                             "True' -- any further steps are undefined behavior.\n";
            }
            m_StepsBeyondDone.value() += 1;
            reward = 0.0;
        }
        timeStep++;
        return {m_State, reward, done, {}};
    }

    void CartPoleEnv::render() {

        [[maybe_unused]] uint const SCREEN_WIDTH{600};
        [[maybe_unused]] uint const SCREEN_HEIGHT{400};

        const auto worldWidth{m_XThreshold * 2};
        const auto scale{SCREEN_WIDTH / worldWidth};
        auto cartY{100.f};
        auto poleWidth{10.0f};
        auto poleLen{scale * (2 * m_Length)};
        auto cartWidth{50.0f};
        auto cartHeight{30.0f};
//        auto axleRadius{poleWidth / 2};

        if(not m_Viewer){

            m_Viewer = std::make_unique<Viewer>(SCREEN_WIDTH, SCREEN_HEIGHT, "CartPole");
            auto[l, r, t, b] = std::make_tuple(-cartWidth / 2, cartWidth / 2, cartHeight / 2, -cartHeight / 2);
            auto axleOffset = cartHeight / 4.0f;

            m_Cart.m_Vertex = {{{l, b}, {l, t}, {r, t}, {r, b}}};
            m_CartTransform = Transform{};
            m_Cart.addAttr(&m_CartTransform);
            m_Viewer->addGeom(&m_Cart);

            std::tie(l, r, t, b) = std::make_tuple(-poleWidth / 2,
                                                   poleWidth / 2,
                                                   poleLen - poleWidth / 2,
                                                   -poleWidth / 2);

            m_Pole.m_Vertex = {{{l, b}, {l, t}, {r, t}, {r, b}}};
            m_Pole.setColor(0.8, 0.6, 0.4);
            m_PoleTransform = Transform{};
            m_PoleTransform.m_Translation = {0, axleOffset};
            m_Pole.addAttr(&m_PoleTransform);
            m_Pole.addAttr(&m_CartTransform);
            m_Viewer->addGeom(&m_Pole);

            m_Axle = std::move(makeFilledCircle(poleWidth / 2, 30));
            m_Axle.addAttr(&m_PoleTransform);
            m_Axle.addAttr(&m_CartTransform);
            m_Axle.setColor(0.5, 0.5, 0.5);
            m_Viewer->addGeom(&m_Axle);

            m_Track.points = {sf::Vector2f{0.f, cartY}, {SCREEN_WIDTH, cartY}};
            m_Track.setColor(0, 0, 0);
            m_Viewer->addGeom(&m_Track);
        }

        auto[l, r, t, b] = std::make_tuple(-poleWidth / 2,
                                               poleWidth / 2,
                                               poleLen - poleWidth / 2,
                                               -poleWidth / 2);
        m_Pole.m_Vertex = {{{l, b}, {l, t}, {r, t}, {r, b}}};

        auto cartX = m_State[0] * scale + SCREEN_WIDTH / 2.0;
        m_CartTransform.m_Translation = {static_cast<float>(cartX), cartY};
        m_PoleTransform.m_Rotation = {static_cast<float>(-m_State[2])};
        m_Viewer->render();

    }
}