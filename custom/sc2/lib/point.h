#pragma once
//
// Created by dewe on 7/15/22.
//
#include "../env/enums.h"

namespace sc2{

    struct Point{

        Point(int _x, int _y):_x(_x), _y(_y){}
        Point(Point const&)=default;
        Point(Point &&) noexcept =default;

        int x() const { return _x; }
        int y() const { return _y; }

        Point& operator=(Point const&) = default;
        Point& operator=(Point &&) noexcept = default;
        Point& operator=(sc_pb::Size2DI const& di) { _x = di.x(); _y = di.y(); }

    private:
        float _x, _y;
    };
}