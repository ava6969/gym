//
// Created by dewe on 10/29/21.
//

#include "utils.h"
namespace gym::box2d::util {
    void CreateEdgeFixture(b2Body *body,
                           std::array<b2Vec2, 2> const &vertices,
                           float density,
                           float friction) {

        b2EdgeShape edgeShape;
        edgeShape.m_vertex0 = vertices[0];
        edgeShape.m_vertex1 = vertices[1];

        b2FixtureDef def;
        def.density = density;
        def.shape = &edgeShape;
        def.friction = friction;

        body->CreateFixture(&def);
    }

    b2Body *createStaticBody(b2World *world, std::array<b2Vec2, 2> const &vertices) {

        b2BodyDef def;
        def.type = b2_staticBody;
        b2Body *body = world->CreateBody(&def);
        CreateEdgeFixture(body, vertices);

        return body;
    }
}