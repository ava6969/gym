#pragma once
//
// Created by dewe on 6/16/22.
//

#include "box2d/box2d.h"
#include "car_racing.h"


namespace gym{

    template<bool>
    class LunarLandarEnv;

    struct FrictionDetector : b2ContactListener{

        explicit FrictionDetector(class CarRacing*);
        void BeginContact(b2Contact* contact) final;
        void EndContact(b2Contact* contact) final;

    private:
        void contact(b2Contact*, bool begin);

        class CarRacing* env;
    };

    template<bool continuous>
    class ContactDetector : public b2ContactListener{

    private:
        LunarLandarEnv<continuous>* m_Env;

    public:
        explicit ContactDetector(LunarLandarEnv<continuous>* env):m_Env(env){}

        void setGroundContact(b2Body* bodyA, b2Body* bodyB, bool value);

        void BeginContact(b2Contact* contact) final;

        void EndContact(b2Contact* contact) final;

    };

    extern template class ContactDetector<true>;
    extern template class ContactDetector<false>;

}