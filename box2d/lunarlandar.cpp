//
// Created by dewe on 8/23/21.
//
#include "contact_listen.h"
#include "common/utils.h"
#include "lunarlandar.h"

namespace gym {

    template<bool cont>
    std::vector<float> LunarLandarEnv<cont>::reset() noexcept {

        destroy();

        m_World->SetContactListener(&contactDetector);
        m_GameOver = false;
        m_PrevShaping = std::nullopt;

        std::vector<float> height = uniformRandom<float>(0, H / 2, CHUNKS + 1, this->m_Device),
                chunkX(CHUNKS),
                smoothY(CHUNKS);

        int i = 0;
        for (auto &ch: chunkX) {
            ch = W / float(CHUNKS - 1) * float(i++);
        }

        m_HelipadX1 = chunkX[floor_div(CHUNKS, 2) - 1];
        m_HelipadX2 = chunkX[floor_div(CHUNKS, 2) + 1];
        m_HelipadY = H / 4;

        height[floor_div(CHUNKS, 2) - 2] = m_HelipadY;
        height[floor_div(CHUNKS, 2) - 1] = m_HelipadY;
        height[floor_div(CHUNKS, 2) + 0] = m_HelipadY;
        height[floor_div(CHUNKS, 2) + 1] = m_HelipadY;
        height[floor_div(CHUNKS, 2) + 2] = m_HelipadY;

        i = 0;
        for (auto &v: smoothY) {
            v = 0.33f * (height[i - 1] + height[i + 0] + height[i + 1]); i++;
        }

        auto edgeShape = box2d::util::edgeShape<2>( { b2Vec2{0.f, 0.f}, {W, 0}} );
        b2Shape* shape = &edgeShape;
        m_Moon.body = box2d::util::CreateStaticBody(*m_World, shape);

        for (int j = 0; j < CHUNKS - 1; j++) {
            sf::Vector2f p1{chunkX[j], smoothY[j]};
            sf::Vector2f p2{chunkX[j + 1], smoothY[j + 1]};

            auto tempShape = box2d::util::edgeShape(
                    std::array<b2Vec2, 2>{b2Vec2{p1[0], p1[1]}, b2Vec2{p2[0], p2[1]}});
            m_Moon.body->CreateFixture( &box2d::util::FixtureDef(&tempShape, 0).friction(0.1).def );

            m_SkyPolys[j] = {p1, p2, {p2[0], H}, {p1[0], H}};

        }

        m_Moon.color = {0.0, 0.0, 0.0};
        m_Moon.color2 = {0.0, 0.0, 0.0};
        m_Moon.userData();

        auto initialY = VIEWPORT_H / SCALE;
        auto landerShape = box2d::util::polygonShape(LANDER_POLY, [](auto p){
            return b2Vec2{p.x/SCALE, p.y/SCALE};
        });

        b2BodyDef landerBodyDef;
        landerBodyDef.position = {VIEWPORT_W / SCALE / 2, initialY};
        landerBodyDef.angle = 0.0;

        m_Lander.body = box2d::util::CreateDynamicBody(*m_World,
                                                  landerBodyDef,
                                                  box2d::util::FixtureDef(&landerShape, 5.0).friction(0.1)
                                                  .restitution(0.0)
                                                  .categoryBits(0x0010)
                                                  .maskBits(0x001).def);
        m_Lander.color = {0.5, 0.4, 0.9};
        m_Lander.color2 = {0.3, 0.3, 0.5};
        m_Lander.userData();

        auto generatedVec = uniformRandom(-INITIAL_RANDOM, INITIAL_RANDOM, 2, this->m_Device);
        m_Lander.body->ApplyForceToCenter({generatedVec.front(), generatedVec.back()}, true);

        float i_ = -1;
        int j = 0;
        for (auto &leg: m_Legs) {
            auto _shape = box2d::util::polygonShapeAsBox(LEG_W / SCALE, LEG_H / SCALE);
            b2BodyDef bodyDef;
            bodyDef.position = {VIEWPORT_W / SCALE / 2.f - i_ * LEG_AWAY / SCALE, initialY};
            bodyDef.angle = i_ * 0.05f;
            leg.body = box2d::util::CreateDynamicBody(*m_World,
                                                      bodyDef,
                                                      box2d::util::FixtureDef(&_shape, 1.0)
                                                      .restitution(0.0)
                                                      .categoryBits(0x0020)
                                                      .maskBits(0x001).def);
            leg.groundContact = false;
            leg.color = {0.5, 0.4, 0.9};
            leg.color2 = {0.3, 0.3, 0.5};
            leg.userData();

            b2RevoluteJointDef rjd;
            rjd.bodyA = m_Lander.body;
            rjd.bodyB = leg.body;
            rjd.localAnchorA = {0, 0};
            rjd.localAnchorB = {i_ * LEG_AWAY / SCALE, LEG_DOWN / SCALE};
            rjd.enableMotor = true;
            rjd.enableLimit = true;
            rjd.maxMotorTorque = LEG_SPRING_TORQUE;
            rjd.motorSpeed = 0.3f * i_;

            if (i_ == -1) {
                rjd.lowerAngle = {0.9 - 0.5};
                rjd.upperAngle = 0.9;
            } else {
                rjd.lowerAngle = -0.9;
                rjd.upperAngle = -0.9 + 0.5;
            }

            leg.body->GetJointList()->joint = m_World->CreateJoint(&rjd);
            i_ += 2;
        }
        m_DrawList = {m_Lander, m_Legs[0], m_Legs[1]};

        if constexpr(cont)
            return step({0.f, 0.f}).observation;
        else
            return step(0).observation;
    }

    template<bool cont>
    StepResponse< std::vector<float> > gym::LunarLandarEnv<cont>::step(const ActionT &_action) noexcept {

        ActionT action = _action;
        if constexpr( cont ) {
            action = { std::clamp(_action[0], -1.f, 1.f), std::clamp(_action[1], -1.f, 1.f) };
        }

        auto tip = std::array<float, 2>{sin(m_Lander.body->GetAngle()), cos(m_Lander.body->GetAngle())};
        auto side = std::array<float, 2>{-tip[1], tip[0]};
        auto dispersion = uniformRandom<float>(-1, 1, 2, this->m_Device) / SCALE;
        auto mPower = 0.f;

        bool power_condition_passed = false;
        if constexpr ( cont ){
            power_condition_passed = action[0] > 0.f;
        }else{
            power_condition_passed = action == 2;
        }

        if (power_condition_passed) {

            if constexpr (cont) {
                mPower = (std::clamp<double>(action[0], 0.0, 1.0) + 1.0) * 0.5F;  // 0.5..1.0
                assert(mPower >= 0.5 and mPower <= 1.0);
            } else {
                mPower = 1.0;
            }
            // 4 is move a bit downwards, +-2 for randomness
            auto ox = tip[0] * (4 / SCALE + 2 * dispersion[0]) + side[0] * dispersion[1];
            auto oy = -tip[1] * (4 / SCALE + 2 * dispersion[0]) - side[1] * dispersion[1];
            auto impulse_pos = b2Vec2{m_Lander.body->GetPosition().x + ox, m_Lander.body->GetPosition().y + oy};

            auto p = createParticle(3.5, impulse_pos.x, impulse_pos.y, mPower);
            p->ApplyLinearImpulse(
                    b2Vec2{static_cast<float>(ox * MAIN_ENGINE_POWER * mPower),
                           static_cast<float>(oy * MAIN_ENGINE_POWER * mPower)},
                    impulse_pos,
                    true
            );

            m_Lander.body->ApplyLinearImpulse(
                    b2Vec2{-ox * MAIN_ENGINE_POWER * mPower,
                           -oy * MAIN_ENGINE_POWER * mPower},
                    impulse_pos,
                    true
            );
        }

        power_condition_passed = false;
        if constexpr ( cont ){
            power_condition_passed = std::abs( action[1] ) > 0.5f;
        }else{
            power_condition_passed = (action == 1 or action == 3);
        }
        auto sPower = 0.f;
        if (power_condition_passed) {

            float direction;
            if constexpr(cont) {
                direction = std::signbit( action[1] ) ? -1 : 1;
                sPower = std::clamp( std::abs( action[1] ), 0.5f, 1.0f);
                assert(sPower >= 0.5 and sPower <=1);
            } else {
                direction = action - 2;
                sPower = 1.f;
            }

            auto ox = tip[0] * dispersion[0] + side[0] * (
                    3 * dispersion[1] + direction * SIDE_ENGINE_AWAY / SCALE
            );

            auto oy = -tip[1] * dispersion[0] - side[1] * (
                    3 * dispersion[1] + direction * SIDE_ENGINE_AWAY / SCALE
            );

            auto impulse_pos = b2Vec2{m_Lander.body->GetPosition().x + ox - tip[0] * 17 / SCALE,
                                      m_Lander.body->GetPosition().y + oy + tip[1] * SIDE_ENGINE_HEIGHT / SCALE};

            auto p = createParticle(0.7, impulse_pos.x, impulse_pos.y, sPower);
            p->ApplyLinearImpulse(
                    {ox * SIDE_ENGINE_POWER * sPower,
                     oy * SIDE_ENGINE_POWER * sPower},
                    impulse_pos,
                    true
            );

            m_Lander.body->ApplyLinearImpulse({-ox * SIDE_ENGINE_POWER * sPower,
                                               -oy * SIDE_ENGINE_POWER * sPower},
                                              impulse_pos, true);
        }

        m_World->Step(1.0 / FPS, 6 * 30, 2 * 30);

        auto const &pos = m_Lander.body->GetPosition();
        auto const &vel = m_Lander.body->GetLinearVelocity();

        std::vector<float> state = {
                (pos.x - VIEWPORT_W / SCALE / 2) / (VIEWPORT_W / SCALE / 2),
                (pos.y - (m_HelipadY + LEG_DOWN / SCALE)) / (VIEWPORT_H / SCALE / 2),
                vel.x * (VIEWPORT_W / SCALE / 2) / FPS,
                vel.y * (VIEWPORT_H / SCALE / 2) / FPS,
                m_Lander.body->GetAngle(),
                20.0f * m_Lander.body->GetAngularVelocity() / FPS,
                m_Legs[0].groundContact ? 1.f : 0.f,
                m_Legs[1].groundContact ? 1.f : 0.f};

        auto reward = 0.f;
        auto shaping = (
                -100 * sqrt(state[0] * state[0] + state[1] * state[1])
                - 100 * sqrt(state[2] * state[2] + state[3] * state[3])
                - 100 * abs(state[4])
                + 10 * state[6]
                + 10 * state[7]);

        if (m_PrevShaping) {
            reward = shaping - m_PrevShaping.value();
        }
        m_PrevShaping = shaping;

        reward -= (mPower * 0.30f);
        reward -= (sPower * 0.03f);

        bool done = false;
        if (m_GameOver or fabs(state[0]) >= 1.0) {
            done = true;
            reward = -100;
        }

        if (not m_Lander.body->IsAwake()) {
            done = true;
            reward = 100;
        }

        return {state, reward, done, {}};
    }

    template<bool cont>
    LunarLandarEnv<cont>::LunarLandarEnv() :
            m_Viewer(nullptr),
            m_World(std::make_unique<b2World>(b2Vec2{0, -10.f})),
            contactDetector(this),
            m_SkyPolys() {

        LunarLandarEnv::seed(std::nullopt);
        m_World->SetAllowSleeping(true);
        m_Particles = {};
        this->m_ObservationSpace = makeBoxSpace<float>(8);

        if constexpr(cont)
            this->m_ActionSpace =  makeBoxSpace<float>({-1}, {1}, 2);
        else
            this->m_ActionSpace = makeDiscreteSpace(4);

        LunarLandarEnv<cont>::reset();
    }

    template<bool cont>
    void LunarLandarEnv<cont>::destroy() noexcept {

        if (not m_Moon.body) {
            return;
        }

        m_World->SetContactListener(nullptr);

        cleanParticle(true);

        m_World->DestroyBody(m_Moon.body);
        m_Moon.body = nullptr;

        m_World->DestroyBody(m_Lander.body);
        m_Lander.body = nullptr;

        m_World->DestroyBody(m_Legs[0].body);
        m_World->DestroyBody(m_Legs[1].body);

        m_Legs[0].body = nullptr;
        m_Legs[1].body = nullptr;
    }

    template<bool cont>
    void LunarLandarEnv<cont>::cleanParticle(bool all) noexcept {
        while (not m_Particles.empty() and
               (all or m_Particles.front().ttl < 0)) {
            m_World->DestroyBody(m_Particles[0].body);
            m_Particles.assign(m_Particles.begin() + 1, m_Particles.end());
        }
    }

    template<bool cont>
    b2Body *gym::LunarLandarEnv<cont>::createParticle(float mass, float x, float y, float ttl) noexcept {

        b2BodyDef bodyDef;
        bodyDef.position = {x, y};
        bodyDef.angle = 0.0;

        b2CircleShape fixtureShape;
        fixtureShape.m_radius = 2 / SCALE;
        fixtureShape.m_p = {0, 0};

        b2FixtureDef fixtureDef;
        fixtureDef.density = mass;
        fixtureDef.friction = 0.1;
        fixtureDef.filter.categoryBits = 0x0100;
        fixtureDef.filter.maskBits = 0x001;
        fixtureDef.restitution = 0.3;
        fixtureDef.shape = &fixtureShape;

        auto p = m_World->CreateBody(&bodyDef);
        p->CreateFixture(&fixtureDef);

        m_Particles.emplace_back(p);
        m_Particles.back().ttl = ttl;
        m_Particles.back().userData();
        cleanParticle(false);
        return p;
    }

    template<bool cont>
    void gym::LunarLandarEnv<cont>::render() {

        if (not m_Viewer) {
            m_Viewer = std::make_unique<Viewer>(VIEWPORT_W, VIEWPORT_H, "LunarLandar");
            m_Viewer->setBounds(0, VIEWPORT_W / SCALE, 0, VIEWPORT_H / SCALE);
        }

        for (auto &obj: m_Particles) {
            obj.ttl -= 0.15;
            obj.color = {
                    std::max(0.2f, 0.2f + obj.ttl),
                    std::max(0.2f, 0.5f + obj.ttl),
                    std::max(0.2f, 0.5f + obj.ttl)
            };

            obj.color2 = obj.color;
        }

        cleanParticle(false);

        Color c{0, 0, 0};
        for (auto const &p: m_SkyPolys) {
            m_Viewer->drawPolygon(p, {&c});
        }

        std::vector<box2d::DrawableBodyBase> particlesAndDrawList;
        particlesAndDrawList.insert(particlesAndDrawList.end(), m_Particles.begin(), m_Particles.end());
        particlesAndDrawList.insert(particlesAndDrawList.end(), m_DrawList.begin(), m_DrawList.end());

        for (auto &obj: particlesAndDrawList) {
            auto *f = obj.body->GetFixtureList();
            while (f) {
                auto trans = f->GetBody()->GetTransform();
                if (auto shape = dynamic_cast<b2CircleShape*>(f->GetShape())) {

                    auto t = trans*shape->m_p;
                    m_Viewer->drawFilledCircle(
                            f->GetShape()->m_radius, 20,
                            {obj.colorAttr()})->addAttr(std::make_unique<Transform>(t));

                    m_Viewer->drawLinedCircle(f->GetShape()->m_radius, 20,
                                              {obj.color2Attr(), obj.lineWidth(2)}
                                              )->addAttr(std::make_unique<Transform>(t));

                } else {
                    auto polygonShape = dynamic_cast<b2PolygonShape *>(f->GetShape());
                    assert(polygonShape);

                    auto path = trans*polygonShape->m_vertices;
                    m_Viewer->drawPolygon(path, {obj.colorAttr()});
                    path.push_back(path[0]);

                    m_Viewer->drawPolyLine(path, {obj.color2Attr(), obj.lineWidth(2)});
                }

                f = f->GetNext();
            }
        }

        for (auto const &x: {m_HelipadX1, m_HelipadX2}) {
            auto flagY1 = m_HelipadY;
            auto flagY2 = flagY1 + 50 / SCALE;

            Color c1(1, 1, 1);
            m_Viewer->drawPolyLine({{x, flagY1},
                                    {x, flagY2}},
                                   {&c1});

            c1 = Color(0.8, 0.8, 0);
            m_Viewer->drawPolygon({{x,              flagY2},
                                   {x,              flagY2 - 10 / SCALE},
                                   {x + 25 / SCALE, flagY2 - 5 / SCALE}},
                                  {&c1});
        }
        m_Viewer->render();
    }

     template class LunarLandarEnv<true>;
     template class LunarLandarEnv<false>;
}


