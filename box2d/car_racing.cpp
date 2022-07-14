//
// Created by dewe on 6/16/22.
//
#include "third_party/GLFont/src/FTLabel.h"
#include "third_party/GLFont/src/FontAtlas.h"
#include "GL/glew.h"
#include <GLFW/glfw3.h>
#include "torch/torch.h"
#include "common/utils.h"
#include "car_racing.h"

namespace gym {

    void CarRacing::render() {
        _render<RenderMode::Human>();
    }

    CarRacing::~CarRacing()=default;

    CarRacing::CarRacing(bool hide, int verbose) :
            contactListener_keepref(nullptr),
            hide_window(hide),
            fd_tile(std::array<std::array<float, 2>, 4>{std::array<float, 2>{0, 0}, {1, 0}, {1, -1}, {0 - 1}}) {

        BaseEnvT::seed(std::nullopt);
        contactListener_keepref = std::make_unique<FrictionDetector>(this);

        this->world = std::make_unique<b2World>(b2Vec2{0, 0.f});
        world->SetContactListener(contactListener_keepref.get());

        viewer = nullptr;

        road.clear();
        car.reset();

        reward = {};
        prev_reward = {};
        this->verbose = verbose;

        m_ActionSpace = makeBoxSpace<float>({-1, 0, 0}, {1, 1, 1}, 3);
        m_ObservationSpace = makeBoxSpace<uint8_t>(0, 255, {STATE_H, STATE_W, 3});

    }

    void gym::CarRacing::destroy() {
        if(road.empty())
            return;
        for(auto const& tile : road){
            world->DestroyBody(tile->body);
//            tile->resetUserData();
//            tile.reset
        }

        road.clear();
        car->destroy();
    }

    cv::Mat CarRacing::reset() noexcept {

        destroy();

        reward = 0.0;
        prev_reward = 0.0;
        tileVisitedCount = 0;
        t = 0.0;
        road_poly.clear();

        while (true) {
            auto success = createTrack();
            if (success)
                break;
            if (verbose)
                std::cout << "retry to generate track (normal if there are not many of this messages)\n";
        }

        car = std::make_optional<box2d::Car>(world.get(), track[0][1], track[0][2], track[0][3]);
        step();

        return state;
    }

    void CarRacing::step() noexcept {

        car->step(1.0 / FPS);
        world->Step(1.0 / FPS, 6 * 30, 2 * 30);
        t += (1.0 / FPS);
        state = _render<RenderMode::StatePixel>();

    }

    StepResponse<cv::Mat> CarRacing::step(const ActionT &action) noexcept {

        car->steer(-action[0]);
        car->gas(action[1]);
        car->brake(action[2]);

        step();

        bool done = false;

        reward -= 0.1;
        car->fuelSpent(0.0);
        auto step_reward = reward - prev_reward;
        prev_reward = reward;

        if (tileVisitedCount == track.size())
            done = true;

        auto [x, y] = car->hullPosition();

        if (abs(x) > PLAYFIELD or abs(y) > PLAYFIELD) {
            done = true;
            step_reward = -100;
        }

        return {state, step_reward, done};
    }

    bool CarRacing::createTrack() {
        using namespace box2d::util;

        constexpr auto CHECKPOINTS = 12;

        std::array<std::array<double, 3>, CHECKPOINTS> checkpoints{};
        float c = 0;
        for (auto &ci: checkpoints) {
            auto alpha = 2 * M_PI * c / CHECKPOINTS + _np_random.uniform(0, 2 * M_PI * 1 / CHECKPOINTS);
            auto rad = _np_random.uniform(TRACK_RAD / 3, TRACK_RAD);
            if (c == 0) {
                alpha = 0;
                rad = 1.5 * TRACK_RAD;
            }
            if (c == CHECKPOINTS - 1) {
                alpha = 2 * M_PI * c / CHECKPOINTS;
                start_alpha = 2 * M_PI * (-0.5) / CHECKPOINTS;
                rad = 1.5 * TRACK_RAD;
            }

            ci = {static_cast<double>(alpha),
                  static_cast<double>(rad * std::cos(alpha)),
                  static_cast<double>(rad * std::sin(alpha))};
            c++;
        }

        road.clear();
        double x = 1.5 * TRACK_RAD, y = 0, beta = 0, laps = 0;
        int dest_i = 0;

        this->track.clear();

        int no_Freeze = 2500;
        bool visited_other_side = false;
        double dest_alpha, dest_x, dest_y;
        while (true) {

            auto alpha = atan2(y, x);
            if (visited_other_side and alpha > 0) {
                laps++;
                visited_other_side = false;
            }
            if (alpha < 0) {
                visited_other_side = true;
                alpha += 2 * M_PI;
            }
            while (true) {
                auto failed = true;
                while (true) {
                    auto r = checkpoints[dest_i % checkpoints.size()];
                    std::tie(dest_alpha, dest_x, dest_y) = std::tie(r[0], r[1], r[2]);
                    if (alpha <= dest_alpha) {
                        failed = false;
                        break;
                    }
                    dest_i++;
                    if (dest_i % checkpoints.size() == 0) {
                        break;
                    }
                }
                if (not failed) {
                    break;
                }
                alpha -= 2 * M_PI;
           }
            double r1x = cos(beta);
            double r1y = sin(beta);
            double p1x = -r1y;
            double p1y = r1x;
            double dest_dx = dest_x - x;
            double dest_dy = dest_y - y;
            double proj = r1x * dest_dx + r1y * dest_dy;
            while (beta - alpha > 1.5 * M_PI)
                beta -= 2 * M_PI;
            while (beta - alpha < -1.5 * M_PI)
                beta += 2 * M_PI;
            double prev_beta = beta;
            proj *= SCALE;
            if (proj > 0.3)
                beta -= std::min<float>(TRACK_TURN_RATE, abs(0.001f * proj));
            if (proj < -0.3)
                beta += std::min<float>(TRACK_TURN_RATE, abs(0.001f * proj));
            x += p1x * TRACK_DETAIL_STEP;
            y += p1y * TRACK_DETAIL_STEP;
            track.push_back(std::vector<double>{static_cast<double>(alpha),
                                                static_cast<double>(prev_beta * 0.5 + beta * 0.5),
                                                static_cast<double>(x),
                                                static_cast<double>(y)});
            if (laps > 4)
                break;
            no_Freeze -= 1;
            if (no_Freeze == 0)
                break;
        }

        int i1 = -1, i2 = -1;
        auto i = int(track.size());
        while (true) {
            i -= 1;
            if (i == 0) {
                return false;
            }
            auto pass_through_start = (track[i][0] > start_alpha) and (track[i - 1][0] <= start_alpha);
            if (pass_through_start and i2 == -1)
                i2 = i;
            else if (pass_through_start and i1 == -1) {
                i1 = i;
                break;
            }
        }

        if (verbose == 1)
            printf("Track generation: %i..%i -> %i-tiles track\n", i1, i2, i2 - i1);

        assert(i1 != -1);
        assert(i2 != -1);

        track.assign(track.begin() + i1, track.begin() + i2 - 1);
        auto N = track.size();

        auto first_beta = track[0][1];
        auto first_perp_x = cos(first_beta), first_perp_y = sin(first_beta);
        auto well_glued_together = sqrt(
                square(first_perp_x * (track[0][2] - track[N - 1][2])) +
                square(first_perp_y * (track[0][3] - track[N - 1][3])));

        if (well_glued_together > TRACK_DETAIL_STEP)
            return false;

        std::vector<bool> border(track.size(), false);

        for (i = 0; i < track.size(); i++) {
            bool good = true;
            double oneside = 0;
            for (int neg = 0; neg < BORDER_MIN_COUNT; neg++) {

                auto beta1 = at(track, i - neg - 0).at(1);
                auto beta2 = at(track, i - neg - 1).at(1);
                auto diff = beta1 - beta2;
                good &= abs(diff) > TRACK_TURN_RATE * 0.2;
                oneside += sign(diff);
            }
            good &= (abs(int(oneside)) == BORDER_MIN_COUNT);
            border[i] = good;
        }

        for (int j = 0; j < track.size(); j++) {
            for (int neg = 0; neg < BORDER_MIN_COUNT; neg++) {
                bool a = at( border, j - neg);
                bool b = border.at(j);
                set( border, j - neg, a | b);
            }
        }

        for (int i = 0; i < track.size(); i++) {
            auto [alpha1, beta1, x1, y1] = std::tie(track[i][0], track[i][1], track[i][2], track[i][3]);
            auto ttrack_i1 = at(track, i - 1);
            auto [alpha2, beta2, x2, y2] = std::tie(ttrack_i1[0], ttrack_i1[1], ttrack_i1[2], ttrack_i1[3]);
            b2Vec2 road1_l = b2Vec2(x1 - TRACK_WIDTH * cos(beta1), y1 - TRACK_WIDTH * sin(beta1)),
                    road1_r = b2Vec2(x1 + TRACK_WIDTH * cos(beta1), y1 + TRACK_WIDTH * sin(beta1)),
                    road2_l = b2Vec2(x2 - TRACK_WIDTH * cos(beta2), y2 - TRACK_WIDTH * sin(beta2)),
                    road2_r = b2Vec2(x2 + TRACK_WIDTH * cos(beta2), y2 + TRACK_WIDTH * sin(beta2));

            std::vector<b2Vec2> vertices({road1_l, road1_r, road2_r, road2_l});
            fd_tile.vertices(vertices);

            auto _t = std::make_unique<box2d::TileBase>( CreateStaticBody(*this->world, fd_tile.def) );
            c = 0.01f * float(i % 3);
            _t->color = {ROAD_COLOR.r + c, ROAD_COLOR.g + c, ROAD_COLOR.b + c};
            _t->road_visited = false;
            _t->road_friction = 1.0;
            _t->body->GetFixtureList()->SetSensor(true);
            _t->setUserData();
            road_poly.emplace_back(std::array<b2Vec2, 4>{road1_l, road1_r, road2_r, road2_l},
                                   std::array<double, 3>{_t->color.vec4[0], _t->color.vec4[1], _t->color.vec4[2]});
            road.emplace_back(std::move(_t));
            if (border[i]) {
                auto side = sign(beta2 - beta1);
                auto b1_l = b2Vec2(x1 + side * TRACK_WIDTH * cos(beta1), y1 + side * TRACK_WIDTH * sin(beta1)),
                        b1_r = b2Vec2(x1 + side * (TRACK_WIDTH + BORDER) * cos(beta1),
                                      y1 + side * (TRACK_WIDTH + BORDER) * sin(beta1)),
                        b2_l = b2Vec2(x2 + side * TRACK_WIDTH * cos(beta2), y2 + side * TRACK_WIDTH * sin(beta2)),
                        b2_r = b2Vec2(x2 + side * (TRACK_WIDTH + BORDER) * cos(beta2),
                                      y2 + side * (TRACK_WIDTH + BORDER) * sin(beta2));
                road_poly.emplace_back(std::array<b2Vec2, 4>{b1_l, b1_r, b2_r, b2_l},
                                       i % 2 == 0 ? std::array<double, 3>{1, 1, 1} : std::array<double, 3>{1, 0, 0});
            }
        }

        return true;
    }

    void CarRacing::renderRoad() {
        glBegin(GL_QUADS);
        glColor4f(0.4, 0.8, 0.4, 1.0);
        glVertex3f(-PLAYFIELD, +PLAYFIELD, 0);
        glVertex3f(+PLAYFIELD, +PLAYFIELD, 0);
        glVertex3f(+PLAYFIELD, -PLAYFIELD, 0);
        glVertex3f(-PLAYFIELD, -PLAYFIELD, 0);
        glColor4f(0.4, 0.9, 0.4, 1.0);
        float k = PLAYFIELD / 20.0f;
        for (int x = -20; x < 20; x += 2) {
            for (int y = -20; y < 20; y += 2) {
                auto kx = k * x;
                auto ky = k * y;
                glVertex3f(kx + k, ky + 0, 0);
                glVertex3f(kx + 0, ky + 0, 0);
                glVertex3f(kx + 0, ky + k, 0);
                glVertex3f(kx + k, ky + k, 0);
            }
        }

        for (auto const &[poly, color]: road_poly) {
            glColor4f(color[0], color[1], color[2], 1);
            for (auto const &p: poly) {
                glVertex3f(p.x, p.y, 0);
            }
        }

        glEnd();
    }

    void CarRacing::renderIndicators(float W, float H) {
        glBegin(GL_QUADS);
        float s = W / 40.0f,
                h = H / 40.0F;
        glColor4f(0, 0, 0, 1);
        glVertex3f(W, 0, 0);
        glVertex3f(W, 5 * h, 0);
        glVertex3f(0, 5 * h, 0);
        glVertex3f(0, 0, 0);

        auto vertical_ind = [s, h](float place, float val, std::array<float, 3> const &color) {
            glColor4f(color[0], color[1], color[2], 1);
            glVertex3f((place + 0.) * s, h + h * val, 0);
            glVertex3f((place + 1.) * s, h + h * val, 0);
            glVertex3f((place + 1.) * s, h, 0);
            glVertex3f((place + 0.) * s, h, 0);
        };

        auto horiz_ind = [s, h](float place, float val, std::array<float, 3> const &color) {
            glColor4f(color[0], color[1], color[2], 1);
            glVertex3f((place + 0.f) * s, 4 * h, 0);
            glVertex3f((place + val) * s, 4 * h, 0);
            glVertex3f((place + val) * s, 2 * h, 0);
            glVertex3f((place + 0.f) * s, 2 * h, 0);
        };

        auto lv = car->hullLinearVelocity();
        auto true_speed = std::sqrt(std::pow(lv[0], 2) + std::pow(lv[1], 2));

        vertical_ind(5, 0.02f * true_speed, {1, 1, 1});
        vertical_ind(7, 0.01f * car->wheels(0)->omega, {0.0, 0, 1}); // ABS sensors
        vertical_ind(8, 0.01f * car->wheels(1)->omega, {0.0, 0, 1});
        vertical_ind(9, 0.01f * car->wheels(2)->omega, {0.2, 0, 1});
        vertical_ind(10, 0.01f * car->wheels(3)->omega, {0.2, 0, 1});
        horiz_ind(20, -10.0f * car->wheels(0)->joint->GetJointAngle(), {0, 1, 0});
        horiz_ind(30, -0.8f * car->hullAngularVelocity(), {1, 0, 0});

        glEnd();

        auto text = "0000"s;
        sprintf(text.data(), "%04i", int(reward));
        score_label->setText(text);
        score_label->render();
    }

    float CarRacing::norm(std::array<float, 2> const &vel) {
        return torch::norm(torch::tensor({vel[0], vel[1]})).template item<float>();
    }

    template<CarRacing::RenderMode m>
    cv::Mat CarRacing::_render() {

        if (not viewer) {
            initRenderer();
        }
        auto p = car->hullPosition();
        float zoom = 0.1 * SCALE * std::max<float>(1 - t, 0) +
                     ZOOM * SCALE * std::min<float>(t, 1),   // Animate zoom first second
        zoom_state = ZOOM * SCALE * STATE_W / WINDOW_W,
                zoom_video = ZOOM * SCALE * VIDEO_W / WINDOW_W,
                scroll_x = p[0],
                scroll_y = p[1],
                angle = -car->hullAngle();

        auto vel = car->hullLinearVelocity();
        if (norm(vel) > 0.5) {
            angle = atan2(vel[0], vel[1]);
        }

        transform.m_Scale = {zoom, zoom};
        transform.m_Translation = {WINDOW_W / 2 - (scroll_x * zoom * cos(angle) - scroll_y * zoom * sin(angle)),
                                   WINDOW_H / 4 - (scroll_x * zoom * sin(angle) + scroll_y * zoom * cos(angle))};
        transform.m_Rotation = angle;

        car->draw<m != RenderMode::StatePixel>(*viewer);

        cv::Mat arr;
        auto win = viewer->win();
        glfwMakeContextCurrent(win);

        if constexpr(m == RenderMode::Human){
            glfwPollEvents();
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int VP_W = 0, VP_H = 0;

        if constexpr(m == RenderMode::RGBArray) {
            VP_W = VIDEO_W;
            VP_H = VIDEO_H;
        } else if constexpr(m == RenderMode::StatePixel) {
            VP_W = STATE_W;
            VP_H = STATE_H;
        } else {
            auto pixel_scale = 1;
            VP_W = int(pixel_scale * WINDOW_W);
            VP_H = int(pixel_scale * WINDOW_H);
        }

        glViewport(0, 0, VP_W, VP_H);
        transform.enable();
        renderRoad();
        auto &otg = viewer->oneTimeGeoms();
        for (auto &geom: otg) {
            geom->render();
        }

        viewer->resetOneTimeGeoms();
        transform.disable();

        renderIndicators(WINDOW_W, WINDOW_H);

        if constexpr(m == RenderMode::Human) {
            glfwSwapBuffers(win);
            return {};
        } else {
            // get cv2 from buffer
            cv::Mat img(VP_H, VP_W, CV_8UC3);
            //use fast 4-byte alignment (default anyway) if possible
            glPixelStorei(GL_PACK_ALIGNMENT, (img.step & 3) ? 1 : 4);

            //set length of one complete row in destination data (doesn't need to equal img.cols)
            glPixelStorei(GL_PACK_ROW_LENGTH, img.step / img.elemSize());
            glReadPixels(0, 0, img.cols, img.rows, GL_BGR, GL_UNSIGNED_BYTE, img.data);
            cv::flip(img, img, 0);
            return img;
        }
    }

    void CarRacing::initRenderer() {
        viewer = std::make_unique<Viewer>(WINDOW_W, WINDOW_H, "car_racing", hide_window);

        std::string home = getenv("HOME");
        _font = make_shared<GLFont>(home.append("/sam/gym/third_party/GLFont/fonts/arial.ttf").c_str());
        auto sY = WINDOW_H * 2.5 / 40.0l;
        score_label = std::make_unique<FTLabel>(_font, "0000", 20 + 36 * 1.5, WINDOW_H - sY - 36,
                                                WINDOW_W, WINDOW_H);
        score_label->setColor(1, 1, 1, 1);
        score_label->setPixelSize(36);
        score_label->setAlignment(FTLabel::FontFlags::CenterAligned);
        transform = Transform();
    }

    template cv::Mat CarRacing::_render<CarRacing::RenderMode::Human>();
    template cv::Mat CarRacing::_render<CarRacing::RenderMode::RGBArray>();
    template cv::Mat CarRacing::_render<CarRacing::RenderMode::StatePixel>();

}