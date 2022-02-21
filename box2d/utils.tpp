//
// Created by dewe on 10/29/21.
//

#ifndef GYMENV_UTIL_TPP
#define GYMENV_UTIL_TPP

#include "box2d/box2d.h"
namespace gym::box2d::util{
    template<typename ShapeType>
    b2Body* CreateDynamicBody(b2World* world,
                           b2Vec2 const& position,
                           ShapeType& shape,
                           float angle,
                           float density,
                           float friction,
                           uint32_t categoryBits,
                           uint32_t maskBits,
                           float restitution){


        b2BodyDef def;
        def.type = b2_dynamicBody;
        def.position.Set(position.x, position.y);
        def.angle = angle;

        b2FixtureDef landerFixtureDef;
        landerFixtureDef.density = density;
        landerFixtureDef.friction = friction;
        landerFixtureDef.restitution=restitution;
        landerFixtureDef.shape = &shape;
        landerFixtureDef.filter.categoryBits = categoryBits;
        landerFixtureDef.filter.maskBits = maskBits;

        b2Body* body = world->CreateBody(&def);
        body->CreateFixture(&landerFixtureDef);
        return body;
    }
}


#endif //GYMENV_UTIL_TPP
