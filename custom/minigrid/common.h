#pragma once
//
// Created by dewe on 6/2/22.
//

#include <cstdint>
#include "unordered_map"
#include "array"
#include "string"
#include "vector"
#include "opencv2/opencv.hpp"


namespace mg {
    constexpr int TILE_PIXELS = 32;
    using Pointf = std::array<float, 2>;

    enum class Color : uint8_t {
        Red = 0, Green, Blue, Purple, Yellow, Grey
    };

    enum class Object : uint8_t {
        Unseen = 0, Empty, Wall, Floor, Door, Key, Ball, Box, Goal, Lava, Agent
    };

    enum class State : uint8_t {
        Open, Closed, Locked
    };

    constexpr const char *COLOR_NAMES[] = {"blue", "green", "grey", "purple", "red", "yellow"};

    static std::string to_string(Color c){
        switch (c) {
            case Color::Red:
                return "RED";
            case Color::Green:
                return "GREEN";
            case Color::Blue:
                return "BLUE";
            case Color::Purple:
                return "PURPLE";
            case Color::Yellow:
                return "YELLOW";
            case Color::Grey:
                return "GREY";
            default:
                return "";
        }

    }

    template<class T>
    constexpr inline uint8_t toIndex(T t) {
        return static_cast<int>(t);
    }

    struct Point {
        int x, y;
    };

    struct Rect {
        Point top_left;
        int width, height;
    };

    using Mask2D = std::vector<std::vector<bool> >;

    template<class T>
    constexpr inline T toEnum(uint8_t index) {
        return static_cast<T>(index);
    }

    static std::unordered_map<Color, std::array<uint8_t, 3>> ColorMap{
            {Color::Red,    {255, 0,   0}},
            {Color::Green,  {0,   255, 0}},
            {Color::Blue,   {0,   0,   255}},
            {Color::Purple, {112, 39,  195}},
            {Color::Yellow, {255, 255, 0}},
            {Color::Grey,   {100, 100, 100}}
    };

    static std::vector<Point> direction{
            {1,  0},
            {0,  1},
            {-1, 0},
            {0,  -1}
    };

    static Point &operator+=(Point &arr, int d) {
        arr.x += d;
        arr.y += d;
        return arr;
    }

    static Point operator+(Point const &arr, int d) {
        auto res = arr;
        res += d;
        return res;
    }

    static Point operator*(Point const &arr, int d) {
        return {arr.x*d, arr.y*d};
    }

    static Point operator-(Point const &arr, int d) {
        auto res = arr;
        res += (-d);
        return res;
    }

    static Point &operator+=(Point &arr, Point const &d) {
        arr.x += d.x;
        arr.y += d.y;
        return arr;
    }

    template<class T>
    static std::array<bool, 2> operator<(Point &arr, T const &d) {
        return {arr.x < d, arr.y < d};
    }

    template<class T>
    static std::array<bool, 2> operator>(Point &arr, T const &d) {
        return {arr.x > d, arr.y > d};
    }

    template<class T>
    static std::array<bool, 2> operator<=(Point &arr, T const &d) {
        return {arr.x <= d, arr.y <= d};
    }

    template<class T>
    static std::array<bool, 2> operator>=(Point &arr, T const &d) {
        return {arr.x >= d, arr.y >= d};
    }


    static Point operator+(Point const &arr, Point const &d) {
        auto res = arr;
        res += d;
        return res;
    }

    static Point operator-(Point const &arr, Point const &d) {
        return {arr.x-d.x, arr.y-d.y};
    }

    static cv::Vec3b toVec3b(std::array<uint8_t, 3> const& c){
        cv::Vec3b res;
        res[0] = c[0];
        res[1] = c[1];
        res[2] = c[2];
        return res;
    }

    static bool operator==(Point const &a, Point const &b) {
        return (a.x == b.x) and (a.y == b.y);
    }

    static std::array<uint8_t, 3> operator/(std::array<uint8_t, 3> const &arr, int const &d) {
        auto result = arr;
        result[0] /= d;
        result[1] /= d;
        result[2] /= d;
        return result;
    }

    static std::array<uint8_t, 3> operator*(std::array<uint8_t, 3> const &arr, float const &d) {
        auto result = arr;
        result[0] = uint8_t(float(result[0]) * d);
        result[1] = uint8_t(float(result[1]) * d);
        result[2] = uint8_t(float(result[2]) * d);
        return result;
    }

    static Pointf operator-(Pointf const &a, Pointf const &b) {
        return {(a[0] - b[0]), (a[1] - b[1])};
    }

    static Pointf operator*(Pointf const &a, Pointf const &b) {
        return {(a[0] * b[0]), (a[1] * b[1])};
    }

    static Pointf operator/(Pointf const &a, Pointf const &b) {
        return {(a[0] / b[0]), (a[1] / b[1])};
    }

    static float dot(Pointf const &a, Pointf const &b) {
        return (a[0] * b[0]) + (a[1] * b[1]);
    }
}