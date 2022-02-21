//
// Created by dewe on 8/25/21.
//

#ifndef GYMENV_UTILS_H
#define GYMENV_UTILS_H

#include "box2d/box2d.h"
#include "array"

namespace gym::box2d::util{
    void CreateEdgeFixture(b2Body* body, std::array<b2Vec2, 2> const& vertices, float density=0, float friction=0.1f);

    b2Body* createStaticBody(b2World* world, std::array<b2Vec2, 2> const& vertices);

    template<typename ShapeType>
    b2Body* CreateDynamicBody(b2World* world,
                           b2Vec2 const& position,
                           ShapeType& shape,
                           float angle=0.0,
                           float density=5.0,
                           float friction=0.2,
                           uint32_t categoryBits=0x0010,
                           uint32_t maskBits=0x001,
                           float restitution=0.f);
}

#include "utils.tpp"

#endif //GYMENV_UTILS_H
