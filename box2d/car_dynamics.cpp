//
// Created by dewe on 6/15/22.
//


#include "car_dynamics.h"

namespace gym::box2d {

    Car::Car(b2World* world, float init_angle, float init_x, float init_y){

        using namespace util;
        m_World = world;

        b2BodyDef def;
        def.position = {init_x, init_y};
        def.angle = init_angle;

        std::array< std::array<float, 2>, 4> v1{}, v2{}, v4{};
        std::array< std::array<float, 2>, 8> v3{};

        for(int i =0; i < 4; i++){
            v1[i] = {HULL_POLY1[i][0]*SIZE, HULL_POLY1[i][1]*SIZE};
            v2[i] = {HULL_POLY2[i][0]*SIZE, HULL_POLY2[i][1]*SIZE};
            v3[i] = {HULL_POLY3[i][0]*SIZE, HULL_POLY3[i][1]*SIZE};
            v4[i] = {HULL_POLY4[i][0]*SIZE, HULL_POLY4[i][1]*SIZE};
        }
        for(int i =4; i < 8; i++){
            v3[i] = {HULL_POLY3[i][0]*SIZE, HULL_POLY3[i][1]*SIZE};
        }

        auto s1 = util::polygonShape(v1), s2 = util::polygonShape(v2), s3 = util::polygonShape(v3),
        s4 = util::polygonShape(v4);

        m_Hull = std::make_unique<DrawableBodyBase>(
                util::CreateDynamicBody(*m_World, def,
                                        std::vector<b2FixtureDef>{ FixtureDef( &s1, 1.0 ).def,
                                                                   FixtureDef( &s2, 1.0 ).def,
                                                                   FixtureDef( &s3, 1.0 ).def,
                                                                   FixtureDef( &s4, 1.0 ).def }));
        m_Hull->colorAttr({0.8, 0, 0});
        constexpr float WHEEL_POLY[][2] = {
                {-WHEEL_W,+WHEEL_R}, {+WHEEL_W,+WHEEL_R},
                {+WHEEL_W,-WHEEL_R}, {-WHEEL_W,-WHEEL_R}
        };

        std::for_each(std::begin(WHEELPOS), std::end(WHEELPOS), [&](auto const& pos){
            auto[wx, wy] = std::tie(pos[0], pos[1]);
            auto front_k = 1.0;

            b2BodyDef def;
            def.position = {init_x + wx*SIZE, init_y + wy*SIZE};
            def.angle = init_angle;

            auto s1 = util::polygonShape(WHEEL_POLY, [front_k, this](auto p){
                return b2Vec2{static_cast<float>(p[0]*front_k*SIZE), static_cast<float>(p[1]*front_k*SIZE)};
            });
            auto f = FixtureDef(&s1, 0.1).categoryBits(0x0020).maskBits(0x001).restitution(0.0).def;
            auto b = CreateDynamicBody(*world, def, f);
            Wheel w = std::make_unique<WheelBase>(b);
            w->wheel_rad = front_k*WHEEL_R*SIZE;
            w->colorAttr(WHEEL_COLOR);

            b2RevoluteJointDef rjd;
            rjd.bodyA = m_Hull->body;
            rjd.bodyB = w->body;
            rjd.localAnchorA = {wx*SIZE, wy*SIZE};
            rjd.localAnchorB = {0, 0};
            rjd.enableMotor = true;
            rjd.enableLimit = true;
            rjd.maxMotorTorque = 180*900*SIZE*SIZE;
            rjd.motorSpeed = 0;
            rjd.lowerAngle = -0.4;
            rjd.upperAngle = 0.4;

            w->joint = dynamic_cast<b2RevoluteJoint *>(world->CreateJoint(&rjd));
            m_Wheels.template emplace_back( std::move(w) );
            m_Wheels.back()->setUserData();
            m_drawList.push_back(m_Wheels.back().get());

        });

        m_drawList.push_back(m_Hull.get());

    }

    void Car::gas(float gas){
        gas = std::clamp(gas, 0.f, 1.f);
        std::for_each(m_Wheels.begin() + 2, m_Wheels.begin() + 4, [gas](auto& w){
            auto diff = gas - w->gas;
            if(diff > 0.1) diff = 0.1;
            w->gas += diff;
        });

    }

    void Car::brake(float b){
        for(auto&w : m_Wheels)
            w->brake = b;
    }

    void Car::steer(float s){
        m_Wheels[0]->steer = s;
        m_Wheels[1]->steer = s;
    }

    void Car::step(float dt){

        for(auto& w : m_Wheels){
            auto joint = dynamic_cast<b2RevoluteJoint*>(w->joint);
            auto dir = sign(w->steer - joint->GetJointAngle());
            auto val = std::abs(w->steer - joint->GetJointAngle());
            joint->SetMotorSpeed( dir*std::min(50.0*val, 3.0));

            bool grass=true;
            double friction_limit = FRICTION_LIMIT*0.6;
            for (auto const& [_, tile]: w->tiles) {
                friction_limit = std::max(friction_limit, FRICTION_LIMIT*tile->road_friction);
                grass = false;
            }

            auto forw = w->body->GetWorldVector( {0, 1} );
            auto side = w->body->GetWorldVector( {1, 0} );
            auto v = w->body->GetLinearVelocity();

            auto vf = forw.x*v.x + forw.y*v.y;
            auto vs = side.x*v.x + side.y*v.y;

            w->omega += dt*ENGINE_POWER*w->gas/WHEEL_MOMENT_OF_INERTIA/(abs(w->omega)+5.0);
            fuel_spent += dt*ENGINE_POWER*w->gas;

            if( w->brake >= 0.9){
                w->omega = 0;
            }else if(w->brake > 0){
                auto BRAKE_FORCE = 15;    // radians per second
                dir = -sign(w->omega);
                val = BRAKE_FORCE*w->brake;
                if ( abs(val) > abs(w->omega)) val = abs(w->omega);  // low speed => same as = 0
                w->omega += dir*val;
            }
            w->phase += w->omega*dt;

            auto vr = w->omega*w->wheel_rad; // rotating wheel speed
            auto f_force = -vf + vr;        // force direction is direction of speed difference
            auto p_force = -vs;

            // Physically correct is to always apply friction_limit until speed is equal.
            // But dt is finite, that will lead to oscillations if difference is already near zero.
            f_force *= 205000*SIZE*SIZE; /// Random coefficient to cut oscillations in few steps (have no effect on friction_limit)
            p_force *= 205000*SIZE*SIZE;
            auto force = std::sqrt( std::pow(f_force, 2) + std::pow(p_force, 2));

            if(abs(force) > 2.0*friction_limit){
                if(w->skid_particle and w->skid_particle->grass and w->skid_particle->poly.size() < 30){
                    auto p = w->body->GetPosition();
                    w->skid_particle->poly.push_back( {p.x, p.y} );
                }else if(not w->skid_start){
                    w->skid_start = w->body->GetPosition();
                }else{
                    w->skid_particle = createParticle( *w->skid_start, w->body->GetPosition(), grass);
                    w->skid_start = std::nullopt;
                }
            }else{
                w->skid_start = std::nullopt;
                w->skid_particle = std::nullopt;
            }

            if(std::abs(force) > friction_limit){
                f_force /= force;
                p_force /= force;
                force = friction_limit;  // Correct physics here
                f_force *= force;
                p_force *= force;
            }

            w->omega -= dt*f_force*w->wheel_rad/WHEEL_MOMENT_OF_INERTIA;

            w->body->ApplyForceToCenter( {p_force*side.x + f_force*forw.x, p_force*side.y + f_force*forw.y}, true );
        }
    }

    template<>
    void Car::draw<true>(Viewer& viewer){
        for(auto& p : particles){
            viewer.drawPolyLine(p.poly, {p.colorAttr(), p.lineWidth()} );
        }
        draw<false>(viewer);
    }

    template<>
    void Car::draw<false>(Viewer& viewer){
        for(DrawableBodyBase* obj: m_drawList){
            auto fixtures = obj->body->GetFixtureList();

            while ( fixtures  ){
                auto trans = fixtures->GetBody()->GetTransform();
                auto shape = dynamic_cast<b2PolygonShape*>(fixtures->GetShape());

                std::vector<std::array<float, 2>> path(shape->m_count);
                for(int i = 0; i < shape->m_count; i++){
                    path[i] = trans * shape->m_vertices[i];
                }

                viewer.drawPolygon(path, {obj->colorAttr()});
                if(obj->phase){
                    auto a1 = *obj->phase;
                    float a2 = a1 + 1.2,
                            s1 = std::sin(a1),
                            s2 = std::sin(a2),
                            c1 = std::cos(a1),
                            c2 = std::cos(a2);

                    if (s1>0 and s2>0) continue;
                    if (s1>0) c1 = sign(c1);
                    if (s2>0) c2 = sign(c2);

                    std::vector<b2Vec2> white_poly ={
                            {-WHEEL_W*SIZE, +WHEEL_R*c1*SIZE},
                            {+WHEEL_W*SIZE, +WHEEL_R*c1*SIZE},
                            {+WHEEL_W*SIZE, +WHEEL_R*c2*SIZE},
                            {-WHEEL_W*SIZE, +WHEEL_R*c2*SIZE}
                    };

                    colorAttr = Color(WHEEL_WHITE.r, WHEEL_WHITE.g, WHEEL_WHITE.b);
                    viewer.drawPolygon(trans*white_poly, {&colorAttr});
                }

                fixtures = fixtures->GetNext();

            }
        }
    }

    Particle& Car::createParticle( b2Vec2 const& point1, b2Vec2 const&  point2, bool grass){
        Particle p;
        p.colorAttr( not grass ? WHEEL_COLOR : MUD_COLOR);
        p.ttl = 1;
        p.poly = { {point1.x, point1.y}, {point2.x, point2.y} };
        p.grass = grass;
        p.line_width.stroke = 5;
        particles.push_back(p);
        while ( particles.size() > 30)
            particles.pop_front();
        return particles.back();
    }

    void Car::destroy() {
        m_World->DestroyBody(m_Hull->body);
        m_Hull.reset();
        for( auto const& w : m_Wheels){
            m_World->DestroyBody(w->body);
//            w->resetUserData();
//            w.reset
        }
        m_Wheels.clear();
    }


} // gym