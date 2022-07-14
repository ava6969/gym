
// Created by dewe on 8/25/21.
//
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <string>
#include "rendering.h"

namespace gym{

    void FilledPolygon::render1() {
        if(m_Vertex.size() == 4){
            glBegin(GL_QUADS);
        }
        else if(m_Vertex.size() > 4){
            glBegin(GL_POLYGON);
        }else{
            glBegin(GL_TRIANGLES);
        }

        for(auto const& point : m_Vertex){
            glVertex3f(point[0], point[1], 0);
        }
        glEnd();
    }

    void Transform::disable() {
        glPopMatrix();
    }

    void Color::enable() {
        glColor4f(vec4[0], vec4[1], vec4[2], vec4[3]);
    }


    void LineStyle::enable() {
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(1, style);
    }


    void LineStyle::disable() {
        glDisable(GL_LINE_STIPPLE);
    }

    void LineWidth::enable() {
        glLineWidth(stroke);
    }

    void PolyLine::render1(){
        glBegin(m_Close ? GL_LINE_LOOP :  GL_LINE_STRIP);
        for(auto const& point : m_Vertex){
            glVertex3f(point[0], point[1], 0);
        }
        glEnd();
    }

    Viewer::Viewer(int width, int height, const std::string &title, bool hide):
            m_Width(width),
            m_Height(height){
        std::lock_guard<std::mutex> lck(mtx);

        if (!glfwInit())
            throw std::runtime_error("glfw failed to fetch");

        auto id = refCount++;
        auto t = title + std::to_string(id);

        if(hide){
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            m_Window = glfwCreateWindow(width, height, "", nullptr, nullptr);
        }else{
            m_Window = glfwCreateWindow(width, height, t.c_str(), nullptr, nullptr);
        }

        if (! m_Window)
        {
            glfwTerminate();
            throw std::runtime_error("glfw failed to create window");
        }

        glfwGetFramebufferSize(m_Window, &width, &height);
        glfwMakeContextCurrent(m_Window);
        glewExperimental=true; // Needed in core profile
        if(glewInit() != GLEW_OK) {
            throw std::runtime_error("Failed to fetch GLEW");
        }

        glfwSetInputMode(m_Window, GLFW_STICKY_KEYS, GL_TRUE);
        glfwSetWindowUserPointer(m_Window, this);

        glViewport(0, 0, std::max(1, width), std::max(1, height));
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, std::max(1, width), 0, std::max(1, height), -1, 1);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    }

    void Viewer::setBounds(float left, float right, float bottom, float top) {
        assert(right > left && top > bottom);

        auto scaleX = (float)m_Width / (right - left);
        auto scaleY = (float)m_Height / (top - bottom);

        m_Transform.m_Translation = {-left * scaleX, -bottom * scaleY};
        m_Transform.m_Scale = {scaleX, scaleY};
    }

    bool Viewer::render() {
//        std::lock_guard<std::mutex> lck(mtx);
        m_IsOpen = glfwWindowShouldClose(m_Window);

        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        m_Transform.enable();

        for (auto* geom : m_Geoms){
            geom->render();
        }

        for (auto const& geom : m_OneTimeGeoms){
            geom->render();
        }

        m_Transform.disable();

        glfwSwapBuffers(m_Window);

        {
            std::lock_guard<std::mutex> lck(mtx);
            glfwPollEvents();
        }

        m_OneTimeGeoms.clear();

        return m_IsOpen;
    }

    Viewer::~Viewer() {
        std::lock_guard<std::mutex> lck(mtx);
        glfwTerminate();
        glfwDestroyWindow(m_Window);
        m_IsOpen = false;
    }

    Geom*  Viewer::drawFilledCircle(float radius, int res,
                                    Attributes const& attrs) {
        std::unique_ptr<Geom> circle = std::make_unique<FilledPolygon>(makeFilledCircle(radius, res));
        return completeAdd(std::move(circle), attrs);
    }

    Geom*  Viewer::drawLinedCircle(float radius, int res, const Attributes &attrs) {
        std::unique_ptr<Geom> circle = std::make_unique<PolyLine>(makeLinedCircle(radius, res));
        return completeAdd(std::move(circle), attrs);
    }

    Geom*  Viewer::drawFilledPolygon(Vertices v, const Attributes &attrs) {
        std::unique_ptr<Geom> poly = std::make_unique<PolyLine>(makeFilledPolygon(std::move(v)));
        return completeAdd(std::move(poly), attrs);
    }

    Geom* Viewer::drawPolygon(Vertices v, const Attributes &attrs) {
        std::unique_ptr<Geom> poly = std::make_unique<FilledPolygon>(std::move(v));
        return completeAdd(std::move(poly), attrs);
    }

    Geom*  Viewer::drawPolyLine(const Vertices &v, const Attributes &attrs) {
        std::unique_ptr<Geom> line = std::make_unique<PolyLine>(v, false);
        return completeAdd(std::move(line),
                           attrs);
    }

    Geom*  Viewer::drawLine(sf::Vector2f const& start,
                            sf::Vector2f const&  end,
                            const Attributes &attrs) {
        std::unique_ptr<Geom> line = std::make_unique<Line>(start, end);
        return completeAdd(std::move(line),
                           attrs);
    }

    Geom *Viewer::completeAdd(std::unique_ptr<Geom> geom,
                              Attributes const& attrs) {
        addAttrs(geom.get(), attrs);
        addOneTime(std::move(geom));
        return m_OneTimeGeoms.back().get();
    }

    void Geom::render() {

        std::for_each(attrs.rbegin(), attrs.rend(), [](auto const& attr){
            attr->enable();
        });

        width.enable();
        color.enable();

        render1();

        std::for_each(attrs.begin(), attrs.end(), [](auto const& attr){
            attr->disable();
        });

        width.disable();
        color.disable();
    }

    Geom &Geom::operator=(Geom &&geom) noexcept {
        this->attrs = std::move(geom.attrs);
        this->color = std::move(geom.color);
        this->width = std::move(geom.width);
        return *this;
    }

    void Geom::addAttr(std::unique_ptr<Attr> &&attr){
        alloc_attrs.emplace_back( std::move(attr) );
        attrs.emplace_back(alloc_attrs.back().get());
    }

    void Compound::render1(){
        for(auto& g : m_Geoms){
            g->render();
        }
    }

    void Point::render1() {
        glBegin(GL_POINTS); // draw point
        glVertex3f(0.0, 0.0, 0.0);
        glEnd();
    }

    Compound makeCapsule(float length, float width){
        auto[l, r, t, b] = std::make_tuple(0.f, length, width / 2, -width / 2.f);

        auto box = std::make_unique<FilledPolygon>();
        box->m_Vertex = {sf::Vector2f{l, b}, sf::Vector2f{l, t}, sf::Vector2f{r, t}, sf::Vector2f{r, b}};

        std::unique_ptr<Geom> circ0, circ1;
        makeLinedCircle(circ0, width / 2);
        makeLinedCircle(circ1, width / 2);

        std::unique_ptr<Attr> ptr = std::make_unique<Transform>(sf::Vector2f{length, 0.f});
        circ1->addAttr( std::move(ptr) );
//
//        return  Compound{move(std::vector<std::unique_ptr<Geom> >{move(box),
//                                                                  move(circ0),
//                                                                  move(circ1)})};
        return Compound{{}};
    }

    PolyLine makeLinedCircle(float radius, int res){
        PolyLine p;
        for(int i = 0; i < res; i++){
            auto ang = 2.f * M_PI * i / res;
            p.m_Vertex.emplace_back(std::array<float, 2>{static_cast<float>(cos(ang) * radius),
                                                         static_cast<float>(sin(ang) * radius)});
        }
        p.m_Close = true;
        return p;
    }

    void makeLinedCircle( std::unique_ptr<Geom>& _p, float radius, int res){
        auto p = std::make_unique<PolyLine>();
        for(int i = 0; i < res; i++){
            auto ang = 2.f * M_PI * i / res;
            p->m_Vertex.emplace_back(std::array<float, 2>{static_cast<float>(cos(ang) * radius),
                                                          static_cast<float>(sin(ang) * radius)});
        }
        p->m_Close = true;
        _p = move(p);
    }

    PolyLine makeFilledPolygon( std::vector<sf::Vector2f> const& v){
        return PolyLine{v, true};
    }

    FilledPolygon makeFilledCircle(float radius, int res){
        FilledPolygon p;
        for(int i = 0; i < res; i++){
            auto ang = 2.f * M_PI * i / res;
            p.m_Vertex.emplace_back(std::array<float, 2>{static_cast<float>(cos(ang) * radius),
                                                         static_cast<float>(sin(ang) * radius)});
        }
        return p;
    }

    void addAttrs(Geom* geom,
                  std::vector<Attr*> const& attrs) {
        for(auto* attr: attrs){
            if( auto* color = dynamic_cast<Color*>(attr)){
                auto[r, g, b, a] = color->vec4;
                geom->setColor(r, g, b);
            }
            else if( auto* linewidth = dynamic_cast<LineWidth*>(attr)){
                geom->setLineWidth(linewidth->stroke);
            }
        }
    }

    void Line::render1(){
        glBegin(GL_LINES);
        glVertex2f(points[0][0], points[0][1]);
        glVertex2f(points[1][0], points[1][1]);
        glEnd();
    }

    Transform &Transform::operator=(Transform &&geom) noexcept {
        this->m_Translation = geom.m_Translation;
        this->m_Scale = geom.m_Scale;
        this->m_Rotation = geom.m_Rotation;
        return *this;
    }

    void Transform::enable() {
        glPushMatrix();
        glTranslatef(m_Translation[0], m_Translation[1], 0);
        glRotatef(RAD2DEG * m_Rotation, 0, 0, 1.0);
        glScalef(m_Scale[0], m_Scale[1], 0);
    }
}