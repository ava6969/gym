//
// Created by dewe on 8/25/21.
//

#ifndef GYMENV_UTILS_H
#define GYMENV_UTILS_H

#include <variant>
#include "optional"
#include "set"
#include "deque"
#include "common/rendering.h"
#include "box2d/box2d.h"
#include "memory"
#include "array"
#include "vector"
#include "common/rendering.h"
#include "common/rendering.h"


static std::array<float, 2> operator*(b2Transform const& tran, b2Vec2 const& v){
    auto result = b2Mul(tran, v);
    return {result.x, result.y};
}

template<class Cont>
static std::vector<std::array<float, 2>> operator*(b2Transform const& tran, Cont const& v){
    std::vector<std::array<float, 2>> result(std::size(v));
    std::ranges::transform(v, result.begin(), [&tran]( auto const&_v){
        return tran*_v;
    });
    return result;
}

namespace gym::box2d{

    using Point = std::array<float, 2>;

    struct Drawable{
        Color color;
        LineWidth line_width{};
        Attr* lineWidth(float lw){
            line_width.stroke = lw;
            return &line_width;
        }

        Attr* lineWidth(){
            return &line_width;
        }

        Attr* colorAttr(){
            return &color;
        }

        Attr* colorAttr(b2Color c){
            color.vec4[0] = c.r;
            color.vec4[1] = c.g;
            color.vec4[2] = c.b;
            color.vec4[3] = c.a;
            return &color;
        }
    };

    struct DrawableBodyBase : Drawable{
        b2Body* body{nullptr};
        std::set<std::string> flags{};
        std::optional<float> phase{0};
        std::unordered_map<std::string, std::variant<int, float, double, bool> > attribs;
        Color color2{};

        DrawableBodyBase()=default;
        explicit DrawableBodyBase(b2Body* body):body(body){}

        void destroyBody(b2World& w){

            body->GetUserData().pointer = 0;
            w.DestroyBody(body);
        }

        virtual void setUserData(){
            body->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);
        }

        Attr* color2Attr(){
            return &color2;
        }

    };
    using DrawableBody = std::unique_ptr<DrawableBodyBase>;

    struct Particle : DrawableBodyBase{
        float ttl{};
        std::vector<Point> poly{};
        bool grass = false;

        Particle(): DrawableBodyBase(nullptr){}
        explicit Particle(b2Body* b): DrawableBodyBase(b){}

        void setUserData() override{
            body->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);
        }
    };

    template<class T>
    static inline auto sign(T v){
        return  v > 0 ? 1  : v < 0 ? -1 : 0;
    }

    using Body = b2Body*;
    using Shape = b2Shape*;
    using Fixture = b2Fixture*;
    using Fixtures = std::vector<Fixture>;

}

namespace gym::box2d::util{

    struct FixtureDef{
        b2FixtureDef def;

        FixtureDef()=default;
        FixtureDef(Shape shape, float density){
            def.shape = shape;
            def.density = density;
        }

        explicit FixtureDef(Shape shape){
            def.shape = shape;
        }

        FixtureDef& vertices(Shape shape){
            def.shape = shape;
            return *this;
        }

        FixtureDef& friction(float f){
            def.friction = f;
            return *this;
        }

        FixtureDef& density(float d){
            def.density = d;
            return *this;
        }

        FixtureDef& categoryBits(uint16_t c){
            def.filter.categoryBits = c;
            return *this;
        }

        FixtureDef& maskBits(uint16_t m){
            def.filter.maskBits = m;
            return *this;
        }

        FixtureDef& restitution(float r){
            def.restitution = r;
            return *this;
        }
    };

    template<class Container, typename ... F>
    static inline b2PolygonShape polygonShape(Container const& vertices, F ... f ){
        static_assert(sizeof ...(F) <= 1, "");
        b2PolygonShape shape;
        const auto N = std::size(vertices);
        b2Vec2 v[N];
        int i = 0;
        std::for_each( std::begin(vertices), std::end(vertices), [&](auto && point){
            if constexpr( sizeof...(F) == 1){
                auto t = std::make_tuple(f ...);
                v[i++] = std::get<0>(t)(point);
            }

            else
                v[i++] = {point[0], point[1]};
        });
        shape.Set( v, N);

        return shape;
    }

    static inline b2PolygonShape polygonShapeAsBox(float hx, float hy){
        b2PolygonShape shape;
        shape.SetAsBox(hx, hy);
        return shape;
    }

    static inline b2PolygonShape polygonShapeAsBox(float hx, float hy, b2Vec2 const& c, float a){
        b2PolygonShape shape;
        shape.SetAsBox(hx, hy, c , a);
        return shape;
    }

    template<size_t N>
    static inline b2EdgeShape edgeShape(std::array<b2Vec2, N> const& vertices){
        b2EdgeShape shape;

        if constexpr(N == 2){
            shape.m_vertex1 = vertices[0];
            shape.m_vertex2 = vertices[1];
        }else if constexpr(N == 3){
            shape.m_vertex0 = vertices[0];
            shape.m_vertex1 = vertices[1];
            shape.m_vertex2 = vertices[2];
        }else if constexpr(N == 4){
            shape.m_vertex0 = vertices[0];
            shape.m_vertex1 = vertices[1];
            shape.m_vertex2 = vertices[2];
            shape.m_vertex3 = vertices[3];
        }else{
            static_assert(true, "edgeShape(vertices) --> Expected from 2 to 4 vertices.");
        }
        return shape;
    }

    template<class T>
    inline auto square(T x){
        return pow(x, 2);
    }

    template<size_t N>
    struct PolygonFixtureDef : FixtureDef{

        PolygonFixtureDef()=default;
        explicit PolygonFixtureDef( std::array< std::array<float, 2>, 4> const& x ):m_Vertices(x){
            s = polygonShape(x);

            FixtureDef::vertices(&s);
        }

        FixtureDef& vertices(std::vector< b2Vec2 > const& v){
            for(int i = 0; i < 4; i++){
                s.m_vertices[i] = v[i];
            }
            return *this;
        }

    private:
        std::array< std::array<float, 2>, 4> m_Vertices;
        b2PolygonShape s;

    };

    template<class T>
    inline typename  T::value_type  at(T const& x, int i){
        return i >= 0 ? x.at(i) : x.at(x.size() + i);
    }

    template<class T, class V>
    inline void set(T& x, int i, V&& v){
        x.at(i >= 0 ? i : x.size() + i) = std::forward<V>(v);
    }

    template <typename T> T sign(T val) { return (T(0) < val) - (val < T(0)); }
    Body createBody(b2World& world, b2BodyDef const& def);
    Body createBody(b2World& world, b2BodyDef const& def, std::vector<b2FixtureDef> const& fixtures);
    Body createBody(b2World& world, b2BodyDef const& def, b2FixtureDef const & fixture);
    Body createBody(b2World& world, b2BodyDef const& def, Shape s);

    Body createBody(b2World& world, b2BodyDef const& def,
                    std::vector<b2FixtureDef> const& fixtures,
                    b2Shape const& shape,
                    b2FixtureDef const& shapeFixture);
    Fixtures createBody(Body body, Shape shape, b2FixtureDef const& shapeFixture);

    Body createBody(b2World& world, b2BodyDef const& def, Shape shape, b2FixtureDef const& shapeFixture);

    Fixtures CreateFixturesFromShapes(Body body, std::vector<Shape> const& shapes);
    Fixtures CreateFixturesFromShapes(Body body, Shape shape);
    Fixtures CreateFixturesFromShapes(Body body, Shape shape, b2FixtureDef const & shapeFixture);

    inline Fixture createFixture(Body body, b2FixtureDef const& def) { return body->CreateFixture(&def); }

    template<class ... Args>
    Body CreateDynamicBody(b2World& world, b2BodyDef def, Args ... args){
        def.type = b2_dynamicBody;
        return createBody(world, def, std::forward<Args>(args) ...);
    }

    template<class ... Args>
    Body CreateKinematicBody(b2World& world, b2BodyDef def, Args ... args){
        def.type = b2_kinematicBody;
        return createBody(world, def, std::forward<Args>(args) ...);
    }

    template<class ... Args>
    Body CreateStaticBody(b2World& world, b2BodyDef def, Args ... args){
        def.type = b2_staticBody;
        return createBody(world, def, std::forward<Args>(args) ...);
    }

    template<class ... Args>
    Body CreateStaticBody(b2World& world, Args ... args){
        b2BodyDef def;
        def.type = b2_staticBody;
        return createBody(world, def, std::forward<Args>(args) ... );
    }
}

#endif //GYMENV_UTILS_H
