//
// Created by dewe on 8/23/21.
//

#include "common/utils.h"
#include "lunarlandar.h"

namespace gym {

    template<bool cont>
    void ContactDetector<cont>::setGroundContact(b2Contact *contact, b2Body *leg, bool value) {
        auto const &bodyA = contact->GetFixtureA()->GetBody();
        auto const &bodyB = contact->GetFixtureB()->GetBody();

        if (leg == bodyA or leg == bodyB) {
            auto &groundContact = reinterpret_cast<UserData *>( leg->GetUserData().pointer )->m_GroundContact;
            groundContact = value;
        }
    }

    template<bool cont>
    void ContactDetector<cont>::BeginContact(b2Contact *contact) {

        auto *bodyA = contact->GetFixtureA()->GetBody();
        auto *bodyB = contact->GetFixtureB()->GetBody();

        if (m_Env->lander() == bodyA or
            m_Env->lander() == bodyB) {
            m_Env->setGameOver();
        }

        setGroundContact(contact, m_Env->leg(0), true);
        setGroundContact(contact, m_Env->leg(1), true);
    }

    template<bool cont>
    void ContactDetector<cont>::EndContact(b2Contact *contact) {
        setGroundContact(contact, m_Env->leg(0), false);
        setGroundContact(contact, m_Env->leg(1), false);
    }

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

        m_HelipadX1 = chunkX[int(CHUNKS / 2) - 1];
        m_HelipadX2 = chunkX[int(CHUNKS / 2) + 1];
        m_HelipadY = H / 4;

        height[int(CHUNKS / 2) - 2] = m_HelipadY;
        height[int(CHUNKS / 2) - 1] = m_HelipadY;
        height[int(CHUNKS / 2) + 0] = m_HelipadY;
        height[int(CHUNKS / 2) + 1] = m_HelipadY;
        height[int(CHUNKS / 2) + 2] = m_HelipadY;

        i = 0;
        for (auto &v: smoothY) {
            v = 0.33f * (height[i - 1] + height[i] + height[i + 1]);
            i++;
        }

        m_Moon = box2d::util::createStaticBody(m_World.get(),
                                               {b2Vec2{0.f, 0.f}, {W, 0}});

        for (int j = 0; j < CHUNKS - 1; j++) {
            sf::Vector2f p1{chunkX[j], smoothY[j]};
            sf::Vector2f p2{chunkX[j + 1], smoothY[j + 1]};
            m_SkyPolys[j] = {p1, p2, {p2[0], H}, {p1[0], H}};

            box2d::util::CreateEdgeFixture(m_Moon,
                                           {b2Vec2{p1[0], p1[1]},
                                            b2Vec2{p2[0], p2[1]}});

        }

        m_Moon->GetUserData().pointer = reinterpret_cast<uintptr_t>(&moonData);
        moonData.m_Color1 = {0.0, 0.0, 0.0};
        moonData.m_Color2 = {0.0, 0.0, 0.0};

        auto initialY = VIEWPORT_H / SCALE;

        b2PolygonShape landerShape;
        std::vector<b2Vec2> points(6);
        i = 0;
        std::transform(std::begin(LANDER_POLY), std::end(LANDER_POLY), points.begin(), [](b2Vec2 const &axis) {
            return b2Vec2{axis.x / SCALE, axis.y / SCALE};
        });
        landerShape.Set(points.data(), 6);
        landerShape.m_count = 6;

        m_Lander = box2d::util::CreateDynamicBody(m_World.get(), {VIEWPORT_W / SCALE / 2, initialY}, landerShape, 0.0,
                                                  5, 0.1);
        lunarLanderData = {b2Color{0.5, 0.4, 0.9}, {0.3, 0.3, 0.5}, false};

        m_Lander->GetUserData().pointer = reinterpret_cast<uintptr_t>(&lunarLanderData);

        auto generatedVec = uniformRandom(-INITIAL_RANDOM, INITIAL_RANDOM, 2, this->m_Device);
        m_Lander->ApplyForceToCenter({generatedVec.front(), generatedVec.back()}, true);

        i = -1;
        int j = 0;
        for (auto &leg: m_Legs) {
            b2PolygonShape legShape;
            legShape.SetAsBox(LEG_W / SCALE, LEG_H / SCALE);
            legShape.m_count = 4;

            leg = box2d::util::CreateDynamicBody(m_World.get(),
                                                 {VIEWPORT_W / SCALE / 2.f - i * LEG_AWAY / SCALE, initialY},
                                                 legShape,
                                                 i * 0.05f,
                                                 1.0f,
                                                 0.2f,
                                                 0x0020,
                                                 0x001);

            legData[j] = {b2Color{0.5, 0.4, 0.9}, {0.3, 0.3, 0.5}, false};
            leg->GetUserData().pointer = reinterpret_cast<uintptr_t>(&legData[j++]);

            b2RevoluteJointDef rjd;
            rjd.bodyA = m_Lander;
            rjd.bodyB = leg;
            rjd.localAnchorA = {0, 0};
            rjd.localAnchorB = {i * LEG_AWAY / SCALE, LEG_DOWN / SCALE};
            rjd.enableMotor = true;
            rjd.enableLimit = true;
            rjd.maxMotorTorque = LEG_SPRING_TORQUE;
            rjd.motorSpeed = 0.3f * i;

            if (i == -1) {
                rjd.lowerAngle = {0.9 - 0.5};
                rjd.upperAngle = 0.9;
            } else {
                rjd.lowerAngle = -0.9;
                rjd.upperAngle = -0.9 + 0.5;
            }

            leg->GetJointList()->joint = m_World->CreateJoint(&rjd);
            i += 2;
        }
        m_DrawList = {m_Lander, m_Legs[0], m_Legs[1]};

        if constexpr(cont)
            return step({0.f, 0.f}).observation;
        else
            return step(1).observation;
    }

    template<bool cont>
    StepResponse< std::vector<float> > gym::LunarLandarEnv<cont>::step(const ActionT &_action) noexcept {

        ActionT action = _action;
        if constexpr( cont ) {
            action = { std::clamp(_action[0], -1, 1), std::clamp(_action[1], -1, 1) };
        }

        auto tip = std::array<float, 2>{sin(m_Lander->GetAngle()), cos(m_Lander->GetAngle())};
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
            auto impulse_pos = b2Vec2{m_Lander->GetPosition().x + ox, m_Lander->GetPosition().y + oy};

            auto p = createParticle(3.5, impulse_pos.x, impulse_pos.y, mPower);
            p->ApplyLinearImpulse(
                    b2Vec2{static_cast<float>(ox * MAIN_ENGINE_POWER * mPower),
                           static_cast<float>(oy * MAIN_ENGINE_POWER * mPower)},
                    impulse_pos,
                    true
            );

            m_Lander->ApplyLinearImpulse(
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

            int direction;
            if constexpr(cont) {
                direction = std::signbit( action[1] ) ? -1 : 1;
                sPower = std::clamp( std::abs( action[1] ), 0.5, 1.0);
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

            auto impulse_pos = b2Vec2{m_Lander->GetPosition().x + ox - tip[0] * 17 / SCALE,
                                      m_Lander->GetPosition().y + oy + tip[1] * SIDE_ENGINE_HEIGHT / SCALE};

            auto p = createParticle(0.7, impulse_pos.x, impulse_pos.y, sPower);
            p->ApplyLinearImpulse(
                    {ox * SIDE_ENGINE_POWER * sPower,
                     oy * SIDE_ENGINE_POWER * sPower},
                    impulse_pos,
                    true
            );

            m_Lander->ApplyLinearImpulse({-ox * SIDE_ENGINE_POWER * sPower,
                                          -oy * SIDE_ENGINE_POWER * sPower},
                                         impulse_pos,
                                         true);
        }

        m_World->Step(1.0 / FPS, 6 * 30, 2 * 30);

        auto const &pos = m_Lander->GetPosition();
        auto const &vel = m_Lander->GetLinearVelocity();

        std::vector<float> state = {
                (pos.x - VIEWPORT_W / SCALE / 2) / (VIEWPORT_W / SCALE / 2),
                (pos.y - (m_HelipadY + LEG_DOWN / SCALE)) / (VIEWPORT_H / SCALE / 2),
                vel.x * (VIEWPORT_W / SCALE / 2) / FPS,
                vel.y * (VIEWPORT_H / SCALE / 2) / FPS,
                m_Lander->GetAngle(),
                20.0f * m_Lander->GetAngularVelocity() / FPS,
                legData[0].m_GroundContact ? 1.f : 0.f,
                legData[1].m_GroundContact ? 1.f : 0.f};

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

        if (not m_Lander->IsAwake()) {
            done = true;
            reward = 100;
        }

        return {state, reward, done, {}};
    }

    template<bool cont>
    LunarLandarEnv<cont>::LunarLandarEnv() :
            m_Viewer(nullptr),
            m_World(std::make_unique<b2World>(b2Vec2{0, -10.f})),
            m_Moon(nullptr),
            m_Lander(nullptr),
            m_SkyPolys() {

        LunarLandarEnv::seed(std::nullopt);
        m_World->SetAllowSleeping(true);
        m_Particles = {};
        this->m_ObservationSpace = makeBoxSpace<float>(8);

        if constexpr(cont)
            this->m_ActionSpace =  makeBoxSpace<float>({-1}, {1}, 2);
        else
            this->m_ActionSpace = makeDiscreteSpace(4);

        reset();
    }

    template<bool cont>
    void LunarLandarEnv<cont>::destroy() noexcept {

        if (not m_Moon) {
            return;
        }

        m_World->SetContactListener(nullptr);

        cleanParticle(true);

        m_World->DestroyBody(m_Moon);
        m_Moon = nullptr;

        m_World->DestroyBody(m_Lander);
        m_Lander = nullptr;

        m_World->DestroyBody(m_Legs[0]);
        m_World->DestroyBody(m_Legs[1]);
    }

    template<bool cont>
    void LunarLandarEnv<cont>::cleanParticle(bool all) noexcept {
        while (not m_Particles.empty() and
               (all or reinterpret_cast<UserData *>( m_Particles[0]->GetUserData().pointer )->ttl < 0)) {
            m_World->DestroyBody(m_Particles[0]);
            m_Particles.assign(m_Particles.begin() + 1, m_Particles.end());
            particleData.assign(particleData.begin() + 1, particleData.end());
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

        particleData.emplace_back();
        particleData.back().ttl = ttl;
        p->GetUserData().pointer = reinterpret_cast<uintptr_t>( &particleData.back());
        m_Particles.push_back(p);
        cleanParticle(false);
        return p;
    }

    template<bool cont>
    void gym::LunarLandarEnv<cont>::render(RenderType) {

        if (not m_Viewer) {
            m_Viewer = std::make_unique<Viewer>(VIEWPORT_W, VIEWPORT_H, "LunarLandar");
            m_Viewer->setBounds(0, VIEWPORT_W / SCALE, 0, VIEWPORT_H / SCALE);
        }

        for (auto const &obj: m_Particles) {
            auto *data = reinterpret_cast<UserData *>(obj->GetUserData().pointer);
            data->ttl -= 0.15;

            data->m_Color1 = {
                    std::max(0.2f, 0.2f + data->ttl),
                    std::max(0.2f, 0.5f + data->ttl),
                    std::max(0.2f, 0.5f + data->ttl)
            };

            data->m_Color2 = data->m_Color1;
        }

        cleanParticle(false);

        for (auto const &p: m_SkyPolys) {
            Color c{0, 0, 0};
            m_Viewer->drawPolygon(p, {&c});
        }

        auto particlesAndDrawList = m_Particles;

        particlesAndDrawList.insert(particlesAndDrawList.end(), m_DrawList.begin(), m_DrawList.end());

        for (auto &obj: particlesAndDrawList) {
            auto *f = obj->GetFixtureList();
            while (f) {
                auto trans = f->GetBody()->GetTransform();
                auto *data = reinterpret_cast<UserData *>(obj->GetUserData().pointer);
                if (f->GetShape()->GetType() == b2Shape::e_circle) {
                    auto circleShape = dynamic_cast<b2CircleShape *>(f->GetShape());
                    assert(circleShape);

                    sf::Vector2f v = {trans.p.x + circleShape->m_p.x,
                                      trans.p.y + circleShape->m_p.y};

                    Color c1 = data->m_Color1;
                    Color c2 = data->m_Color1;

                    m_Viewer->drawFilledCircle(
                            f->GetShape()->m_radius, 20,
                            {&c1})->addAttr(std::make_unique<Transform>(v));

                    m_Viewer->drawLinedCircle(f->GetShape()->m_radius, 20,
                                              {&c2})->addAttr(std::make_unique<Transform>(v));

                } else {
                    auto polygonShape = dynamic_cast<b2PolygonShape *>(f->GetShape());
                    assert(polygonShape);

                    Vertices path;

                    std::transform(std::begin(polygonShape->m_vertices),
                                   std::begin(polygonShape->m_vertices) + polygonShape->m_count,
                                   std::back_inserter(path), [&trans](b2Vec2 v) {
                                return sf::Vector2f{v.x + trans.p.x, v.y + trans.p.y};
                            });

                    Color c = data->m_Color1;
                    m_Viewer->drawPolygon(path, {&c});

                    path.push_back(path[0]);

                    Color c2 = data->m_Color2;
                    LineWidth l(2);

                    m_Viewer->drawPolyLine(path, {&c2, &l});
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
}


