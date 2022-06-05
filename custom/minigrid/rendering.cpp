//
// Created by dewe on 6/3/22.
//

#include <cmath>
#include "opencv2/opencv.hpp"
#include "torch/torch.h"
#include "rendering.h"

namespace mg {
    void downsample(cv::Mat &img, int factor) {

        auto w = img.size[0];
        auto h = img.size[1];
        auto c = img.channels();

        if (w % factor == 0 and h % factor == 0) {
            auto t_img = torch::from_blob(img.data, {w * h * c}, c10::kChar).view({w, h, c});
            w /= factor;
            h /= factor;
            t_img = t_img.view({w , factor, h , factor, 3}).to(c10::kFloat);
            t_img = t_img.mean(3).mean(1).view({w, h, 3}).flatten().to(torch::kUInt8);

            img = cv::Mat( w, h, CV_8UC3 );
            memmove(img.data, t_img.data_ptr(), w*h*3);

        } else {
            throw std::runtime_error("Inorder to down sample, Img must be a multiple of scale factor");
        }
    }

    void fill_coords(cv::Mat &img, std::function<bool(float, float)> const &fn, std::array<uint8_t, 3> const &color) {
        auto w = img.size[0];
        auto h = img.size[1];

        for (int y = 0; y < w; y++) {
            for (int x = 0; x < h; x++) {
                auto yf = (float(y) + 0.5f) / (float) w;
                auto xf = (float(x) + 0.5f) / (float) h;
                if (fn(xf, yf))
                    img.at<cv::Vec3b>(y, x) = toVec3b(color);
            }
        }
    }

    std::function<bool(float, float)> point_in_rect(Pointf const &a, Pointf const &b) {
        return [=](float x, float y) {
            return x >= a[0] and x <= a[1] and y >= b[0] and y <= b[1];
        };
    }

    std::function<bool(float, float)> point_in_line(Pointf const &a, Pointf const &b, float r) {
        auto p0 = torch::tensor({a[0], a[1]});
        auto p1 = torch::tensor({b[0], b[1]});
        auto dir = p1 - p0;
        auto dist = dir.norm();
        dir = dir / dist;

        auto xmin = std::min<float>(a[0], b[0]) - r;
        auto xmax = std::max<float>(a[0], b[0]) + r;
        auto ymin = std::min<float>(a[1], b[1]) - r;
        auto ymax = std::max<float>(a[1], b[1]) + r;

        return [=](float x, float y) {
            if (x < xmin or x > xmax or y < ymin or y > ymax)
                return false;
            auto q = torch::tensor({x, y});
            auto pq = q - p0;

            auto a = std::clamp(pq.dot(dir).item<float>(), 0.f, dist.item<float>());
            auto p = p0 + a * dir;

            auto dist_to_line = (q - p).norm().item<float>();
            return (dist_to_line <= r);
        };
    }

    std::function<bool(float, float)> point_in_circle(float cx, float cy, float r) {
        return [=](float x, float y) {
            return (x - cx) * (x - cx) + (y - cy) * (y - cy) <= r * r;
        };
    }

    std::function<bool(float, float)> point_in_triangle(Pointf const &a,
                                                        Pointf const &b,
                                                        Pointf const &c) {

        return [=](float x, float y) {
            auto v0 = c - a;
            auto v1 = b - a;
            auto v2 = Pointf{x, y} - a;

            auto dot00 = dot(v0, v0);
            auto dot01 = dot(v0, v1);
            auto dot02 = dot(v0, v2);
            auto dot11 = dot(v1, v1);
            auto dot12 = dot(v1, v2);

            auto inv_denom = 1 / (dot00 * dot11 - dot01 * dot01);
            auto u = (dot11 * dot02 - dot01 * dot12) * inv_denom;
            auto v = (dot00 * dot12 - dot01 * dot02) * inv_denom;

            return (u >= 0) and (v >= 0) and (u + v) < 1;
        };
    }

    std::function<bool(float, float)> rotate_fn(std::function<bool(float, float)> const &fin,
                                                Pointf const& c, float theta) {
        return [=](float x, float y) {
            x -= c[0];
            y -= c[1];
            auto x2 = c[0] + x * cos(-theta) - y * sin(-theta);
            auto y2 = c[1] + y * cos(-theta) + x * sin(-theta);
            return fin(x2, y2);
        };
    }

    void highlight_img(cv::Mat &img,
                       std::array<uint8_t, 3> const &color,
                       float alpha) {

        auto solid_color = cv::Mat(img.size[0], img.size[1], CV_8UC3, cv::Scalar_(color[0], color[1], color[2]) );
        cv::Mat diff = solid_color - img;

        diff.convertTo(diff, CV_32F);
        img.convertTo(img, CV_32F);

        cv::Mat blend_img = img + alpha*diff;
        img = blend_img.setTo(255, blend_img > 255).setTo(0, blend_img < 0);
        img.convertTo(img, CV_8UC3);
    }
}