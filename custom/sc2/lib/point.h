#pragma once
//
// Created by dewe on 7/15/22.
//
#include "../env/enums.h"

namespace sc2{

    struct Point{
        Point()=default;
        explicit Point(int x):_x(x), _y(x){}
        Point(int _x, int _y):_x(_x), _y(_y){}
        Point(Point const&)=default;
        Point(Point &&) noexcept =default;

        int x() const { return _x; }
        int y() const { return _y; }

        Point& operator=(Point const&) = default;
        Point& operator=(Point &&) noexcept = default;
        Point& operator=(sc_pb::Size2DI const& di) { _x = di.x(); _y = di.y(); return *this; }
        bool operator==(Point const&) const =default;

        friend std::ostream& operator<<( std::ostream& os, Point const& dim){
            os << "Point(x=" << dim._x << ", y=" << dim._y << ")";
            return os;
        }

        template<typename T>
        void assign_to(T* di) const { di->set_x(_x); di->set_y(_y); }

        friend class Rect;
    private:
        int _x{0}, _y{0};
    };

    const Point ORIGIN{0, 0};

    struct Rect{

        explicit Rect(Point const p): Rect(ORIGIN, p){}

        Rect(Point const p1, Point const& p2):
        m_t( std::min(p1._y, p2._y)),
        m_l( std::min(p1._x, p2._x)),
        m_b( std::max(p1._y, p2._y)),
        m_r( std::max(p1._x, p2._x)){}

        Point tl() const { return {m_l, m_t}; }
        Point br() const { return {m_r, m_b}; }
        Point tr() const { return {m_r, m_t}; }
        Point bl() const { return {m_l, m_b}; }

    private:
        int m_t, m_l, m_b, m_r;
    };
}