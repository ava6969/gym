//
// Created by dewe on 6/16/22.
//

#include "car_racing.h"
#include "car_dynamics.h"
#include "lunarlandar.h"
#include "contact_listen.h"

namespace gym{
    FrictionDetector::FrictionDetector( CarRacing* env):env(env){}

    void FrictionDetector::BeginContact(b2Contact* _contact) {
        contact(_contact, true);
    }

    void FrictionDetector::EndContact(b2Contact* _contact) {
        contact(_contact, false);
    }


    void FrictionDetector::contact(b2Contact* _contact, bool begin){

        using namespace box2d;
        TileBase* tile{nullptr};
        WheelBase* obj{nullptr};

        auto u1 = _contact->GetFixtureA()->GetBody()->GetUserData().pointer;
        auto u2 = _contact->GetFixtureB()->GetBody()->GetUserData().pointer;

        auto complete = [this](TileBase* t, WheelBase* w, bool begin){
            t->color.vec4[0] = CarRacing::ROAD_COLOR.r;
            t->color.vec4[1] = CarRacing::ROAD_COLOR.g;
            t->color.vec4[2] =CarRacing:: ROAD_COLOR.b;

            if(begin ){
                w->add(t);

                if( not t->road_visited){
                    t->road_visited = true;
                    env->reward += (1000.0/env->track.size());
                    env->tileVisitedCount += 1;
                }
            }else{
                w->remove(t);
            }

        };

        TileBase* temp_tile;
        if( (temp_tile = reinterpret_cast<TileBase*>(u1)) ){

            tile = temp_tile;
            obj = reinterpret_cast<WheelBase*>(u2);

            if(not obj)
                return;

        }else  if( (temp_tile = reinterpret_cast<TileBase*>(u2)) ){

            tile = temp_tile;
            obj = reinterpret_cast<WheelBase*>(u1);

            if(not obj)
                return;
        }else{
            return;
        }

        complete(tile, obj, begin);

    }

    template<bool cont>
    void ContactDetector<cont>::setGroundContact(box2d::Body bodyA, box2d::Body bodyB, bool value) {
        auto bodies = std::set{bodyA, bodyB};
        for(int i = 0; i < 2; i++){
            if( bodies.contains( m_Env->leg(i).body ) ){
                m_Env->leg(i).groundContact = value;
            }
        }
    }

    template<bool cont>
    void ContactDetector<cont>::BeginContact(b2Contact *contact) {

        auto *bodyA = contact->GetFixtureA()->GetBody();
        auto *bodyB = contact->GetFixtureB()->GetBody();

        if (m_Env->lander() == bodyA or m_Env->lander() == bodyB) {
            m_Env->setGameOver();
        }

        setGroundContact(bodyA, bodyB, true);
    }

    template<bool cont>
    void ContactDetector<cont>::EndContact(b2Contact *contact) {
        setGroundContact(contact->GetFixtureA()->GetBody(),
                         contact->GetFixtureB()->GetBody(),
                         false);
    }

    template class ContactDetector<true>;
    template class ContactDetector<false>;
}



