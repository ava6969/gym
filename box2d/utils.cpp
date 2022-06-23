//
// Created by dewe on 10/29/21.
//

#include "utils.h"



namespace gym::box2d::util {

    b2Body* createBody(b2World&  world, b2BodyDef const& def){
        return world.CreateBody(&def);
    }

    b2Body* createBody(b2World& world, b2BodyDef const& def,
                       std::vector<b2FixtureDef> const& fixtures){
        auto body = createBody(world, def);
        for (auto const& fixture: fixtures) {
            body->CreateFixture(&fixture);
        }
        return body;
    }

    Body createBody(b2World& world, b2BodyDef const& def, b2FixtureDef const & fixture){
        auto body = createBody(world, def);
        body->CreateFixture(&fixture);
        return body;
    }

    Fixtures createBody(Body body, Shape shape, b2FixtureDef const& shapeFixture){
        return CreateFixturesFromShapes(body, shape, shapeFixture);
    }

    Body createBody(b2World& world, b2BodyDef const& def, Shape shape, b2FixtureDef const& shapeFixture){
        auto body = createBody(world, def);
        createBody(body, shape, shapeFixture);
        return body;
    }

    Body createBody(b2World& world, b2BodyDef const& def, Shape shape){
        auto body = createBody(world, def);
        createBody(body, shape, {});
        return body;
    }

    Fixtures CreateFixturesFromShapes(Body body, std::vector<Shape> const& shapes){
        std::vector<b2Fixture*> ret(shapes.size());

        std::transform(shapes.begin(), shapes.end(), ret.begin(),
                       [&body](Shape shape){
                           b2FixtureDef shapeFixture;
                            shapeFixture.shape = shape;
                           return body->CreateFixture(&shapeFixture);
        });

        return ret;

    }

    Fixtures CreateFixturesFromShapes(Body body, Shape shape){
        b2FixtureDef shapeFixture;
        return CreateFixturesFromShapes(body, shape, shapeFixture);
    }

    Fixtures CreateFixturesFromShapes(Body body, Shape shape, b2FixtureDef const & shapeFixture){
        auto oldShape = shapeFixture.shape;
        const_cast<b2FixtureDef&>(shapeFixture).shape = shape;
        auto ret = {body->CreateFixture(&shapeFixture)};
        const_cast<b2FixtureDef&>(shapeFixture).shape = oldShape;
        return ret;
    }
}