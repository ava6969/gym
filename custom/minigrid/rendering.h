#pragma once
//
// Created by dewe on 6/3/22.
//
#include "array"
#include "cinttypes"
#include "functional"
#include "common.h"


namespace cv{
    class Mat;
}

namespace mg {

    void downsample(cv::Mat &img, int factor);

    void fill_coords(cv::Mat &, std::function<bool(float, float)> const &, std::array<uint8_t, 3> const &);

    std::function<bool(float, float)> point_in_rect(Pointf const &a, Pointf const &b);

    std::function<bool(float, float)> point_in_line(Pointf const &a, Pointf const &b, float r);

    std::function<bool(float, float)> point_in_circle(float cx, float cy, float r);

    std::function<bool(float, float)> point_in_triangle(Pointf const &a, Pointf const &b, Pointf const &c);

    std::function<bool(float, float)> rotate_fn(std::function<bool(float, float)> const &fin,
                                                Pointf const &c, float theta);

    void highlight_img(cv::Mat&img,
                       std::array<uint8_t, 3> const &color = {255, 255, 255},
                       float alpha = 0.3);
}