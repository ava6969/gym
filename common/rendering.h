//
// Created by dewe on 8/23/21.
//

#ifndef GYMENV_RENDERING_H
#define GYMENV_RENDERING_H


#include <array>
#include <utility>
#include <vector>
#include <memory>
#include <cmath>
#include <cassert>
#include <mutex>
#include <iostream>
#include "unordered_map"
#include "box2d/box2d.h"
#include "algorithm"


#ifndef M_PI
#define M_PI 3.141592653589793238
#endif

namespace sf{
    using Vector2f = std::array<float, 2>;
}

namespace gym{

    enum class AttributeType{
        Transform,
        Color,
        LineStyle,
    };

    using Vertices = std::vector<sf::Vector2f>;
    using Attributes = std::vector<struct Attr*>;

    const static float RAD2DEG = 57.29577951308232;

    struct Attr{
        AttributeType type;
        virtual void enable() = 0;
        virtual void disable() {};
        virtual ~Attr()=default;
    };

    struct Transform : Attr {
        sf::Vector2f m_Translation{},m_Scale{1, 1};
        float m_Rotation{0};

        Transform(sf::Vector2f translation):m_Translation(translation){}
        Transform()=default;
        Transform& operator=(Transform&& geom) noexcept;

        void enable();
        void disable();
    };

    struct Color : Attr{
        std::array<float, 4> vec4{0, 0, 0, 1};

        Color()=default;
        explicit Color(b2Color c):
                vec4({c.r, c.g, c.b, c.a}){}

        Color(float r, float g, float b):
                vec4({r, g, b, 1}){};

        void enable() override;
    };

    struct LineStyle : Attr{
        short style;

        void enable() override;

        void disable() override;
    };

    struct LineWidth : Attr {
        float stroke{};

        LineWidth()=default;
        LineWidth(float _stroke):stroke(_stroke) {}

        void enable() override;
    };

    struct Geom{
        Color color{};
        LineWidth width{};
        std::vector< std::unique_ptr<Attr> > alloc_attrs{};
        std::vector< Attr* > attrs{};

        Geom(){
            color.vec4 = {0, 0, 0, 1};
        }

        Geom(Geom&& _clone) noexcept :
                color(std::move(_clone.color)),
                width(std::move(_clone.width)),
                attrs(std::move(_clone.attrs)) {}

        void render();

        virtual void render1(){}

        Geom& operator=(Geom&& geom) noexcept;

        void addAttr(std::unique_ptr<Attr>&& attr);

        inline void addAttr(Attr* attr){
            attrs.emplace_back(attr);
        }

        void setColor(float r, float g, float b){
            color.vec4 = {r, g, b, 1};
        }

        void setLineWidth(short stroke){
            width.stroke = stroke;
        }

        virtual ~Geom()=default;
    };

    struct Point : Geom{
        void render1() override;
    };

    struct FilledPolygon : Geom{
        std::vector<sf::Vector2f> m_Vertex{};

        explicit FilledPolygon(Vertices v):m_Vertex(std::move(v)){}
        FilledPolygon()=default;

        void render1() override;
    };

    struct PolyLine : Geom{
        std::vector<sf::Vector2f> m_Vertex{};
        bool m_Close{};

        PolyLine()=default;
        PolyLine( std::vector<sf::Vector2f> vertices, bool close):
                m_Vertex(move(vertices)),
                m_Close(close){}

        void render1() override;
    };

    struct Line : Geom{
        std::array<sf::Vector2f, 2> points{};

        Line(){
            addAttr(std::make_unique<LineWidth>(1));
        }

        Line(sf::Vector2f const& start, sf::Vector2f const& end){
            points = {start, end};
            addAttr(std::make_unique<LineWidth>(1));
        }

        void render1() override;
    };

    struct Compound : Geom{
    private:
        std::vector< std::unique_ptr<Geom> > m_Geoms;

    public:
        explicit Compound(std::vector<std::unique_ptr<Geom>> geoms):
                m_Geoms(move(geoms)){

            for(auto const& geom : m_Geoms){
               auto removed = std::remove_if( geom->attrs.begin(), geom->attrs.end(), [&geom](auto const&){
                                                   return dynamic_cast<Color*>(geom.get());
                                               });
//#ifndef WIN32
//               if(! removed ){
//                   throw std::runtime_error("Failed to Remove Geom\n");
//               }
//#endif
            }
        }

        void render1() override;
    };

    class Viewer{

        inline static int refCount = 0;
        inline static std::mutex mtx;
    public:
        explicit Viewer(int width, int height, std::string const& title, bool hide=false);

        void setBounds(float left, float right, float bottom, float top);

        inline
        void addGeom(Geom* geom){
            m_Geoms.push_back(geom);
        }

        inline
        void addOneTime(std::unique_ptr<Geom> geom){
            m_OneTimeGeoms.push_back(std::move(geom));
        }

        bool render();

        Geom* drawFilledCircle(float radius=10, int res=30,
                               Attributes const& attrs={});
        Geom* drawLinedCircle(float radius=10, int res=30,
                              Attributes const& attrs={});

        Geom* drawFilledPolygon(Vertices v, Attributes const& attrs={});

        Geom* drawPolygon(Vertices v, Attributes const& attrs={});

        Geom* drawPolyLine(Vertices const& v, Attributes const& attrs={});

        Geom*          drawLine(sf::Vector2f const&  start,
                                sf::Vector2f const&  end,
                                Attributes const& attrs={});

        [[nodiscard]] inline auto& oneTimeGeoms() const { return m_OneTimeGeoms; }
        inline void resetOneTimeGeoms()  {  m_OneTimeGeoms.clear(); }
        inline auto win() { return m_Window; }

        ~Viewer();

    private:
        class  GLFWwindow* m_Window;
        bool m_IsOpen;
        int m_Width, m_Height;

        std::vector<Geom*> m_Geoms{};
        std::vector<std::unique_ptr<Geom > >m_OneTimeGeoms{};
        Transform m_Transform{{}};

        Geom* completeAdd(std::unique_ptr<Geom>, Attributes const&);
    };

    void addAttrs(Geom* geom, std::vector<Attr*> const& attrs);

    FilledPolygon makeFilledCircle(float radius, int res);

    PolyLine makeLinedCircle(float radius, int res);

    void makeLinedCircle( std::unique_ptr<Geom>& _p, float radius=10, int res=30);

    PolyLine makeFilledPolygon( std::vector<sf::Vector2f> const& v);

    Compound makeCapsule(float length, float width);
}


#endif //GYMENV_RENDERING_H