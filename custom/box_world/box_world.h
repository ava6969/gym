//
// Created by Ben Giacalone on 6/30/2021.
//

#ifndef GYMENV_BOX_WORLD_H
#define GYMENV_BOX_WORLD_H

#include "env.h"
#include "spaces/box.h"
#include "spaces/discrete.h"
#include "common/rendering.h"
#include "opencv2/core.hpp"
#include "../minigrid/common.h"
#include "torch/torch.h"
#include "span"


#define BoxColor(...) cv::Vec3b({ __VA_ARGS__ })
using BColor = cv::Vec3b;

namespace gym {

    /**
     * A CPP implementation of the Box World environment, based on https://github.com/mavischer/Box-World/tree/master/gym-boxworld
     *
     * The observation space is a Box space of (n + 2) x (n + 2) x 3.
     * The action space is a Discrete space of 4:
     * 0:                   move up
     * 1:                   move down
     * 2:                   move left
     * 3:                   move right
     *
     * The following config parameters are available:
     * n:          size of the board
     * goal_length:         length of the goal path
     * num_distractor:      number of distractor branches
     * distractor_length:   length of each distractor branch
     * step_cost:           reward per step
     * reward_gem:          reward for getting a gem
     * reward_dead:         reward for dying
     * reward_correct_key   reward for getting a correct key
     * reward_wrong_key     reward for getting a wrong key
     * max_steps            maximum number of steps before terminating
     */
class BoxWorld : public Env< cv::Mat, int> {
    private:

        // Constants

        static constexpr int SCALE = 40;
        static constexpr int MAX_COLORS = 20;
        inline static const cv::Vec3b AGENT_COLOR = BoxColor(128, 128, 128);
        inline static const cv::Vec3b GOAL_COLOR = BoxColor(255, 255, 255);
        inline static const cv::Vec3b BACKGROUND_COLOR = BoxColor(220, 220, 220);
        inline static const cv::Vec3b COLORS[MAX_COLORS] = {
                BoxColor(0, 0, 117),
                BoxColor(230, 190, 255),
                BoxColor(170, 255, 195),
                BoxColor(255, 250, 200),
                BoxColor(255, 216, 177),
                BoxColor(250, 190, 190),
                BoxColor(240, 50, 230),
                BoxColor(145, 30, 180),
                BoxColor(67, 99, 216),
                BoxColor(66, 212, 244),
                BoxColor(60, 180, 75),
                BoxColor(191, 239, 69),
                BoxColor(255, 255, 25),
                BoxColor(245, 130, 49),
                BoxColor(230, 25, 75),
                BoxColor(128, 0, 0),
                BoxColor(154, 99, 36),
                BoxColor(128, 128, 0),
                BoxColor(70, 153, 144),
                BoxColor(100, 70, 0)
        };

        static inline std::array<mg::Point, 4> CHANGE_COORDINATES = {
                mg::Point{-1, 0},
                mg::Point{1,  0},
                mg::Point{0,  -1},
                mg::Point{0,  1}
        };

        // Environment properties
        int n;
        int m_NumPairs;
        int m_NumColors;
        std::vector<cv::Vec3b> m_Colors;
        int m_MaxSteps;
        cv::Mat state;

        float m_StepCost;
        float m_RewardGem;
        float m_RewardDead;
        float m_RewardCorrectKey;
        float m_RewardWrongKey;

        // Used during environment execution
        int m_NumEnvSteps;
        cv::Mat m_World;
        mg::Point m_PlayerPosition;
        std::vector<BColor> m_DeadEnds;
        std::vector<BColor> m_CorrectKeys;
        BColor m_OwnedKey;

        // Rendering variables
        std::unique_ptr<class Viewer> m_Viewer;
        std::vector<std::vector<sf::Vector2f>> m_GridVertices;

        // Checks if a space is empty.
        static bool is_empty(cv::Vec3b const& space);

        // Updates the color of a space after an agent moves.
        static void update_color(cv::Mat & world, mg::Point const&, mg::Point const& new_pos);

        // Generates random key,lock pairs locations in the environment.
        std::tuple< std::vector<mg::Point>, std::vector<mg::Point>, mg::Point, mg::Point>
                sample_pair_locations(int num_pair);

        inline auto& worldAt(mg::Point const& x) { return m_World.at<BColor>(x.x, x.y); }
        inline auto worldAtC(mg::Point const& x) { return m_World.at<BColor>(x.x, x.y); }

    protected:
        int m_GoalLength;
        int m_NumDistractor;
        int m_DistractorLength;
        bool verbose;

        // Generates world from a seed.
        virtual std::tuple<cv::Mat, mg::Point, std::vector<BColor>, std::vector<BColor>>  world_gen();

        template<typename T>
        inline auto slice(std::vector<T>& x, int i, int j){
            return std::span<T>(x.begin() + i, x.begin() +  j);
        }

    public:

        struct Option{
            int n=12,
             goal_length=5,
             num_distractor=2,
             distractor_length=2,
             max_steps=3000,
             num_colors=MAX_COLORS;
            float step_cost=0,
             reward_gem=10,
             reward_dead=0,
             reward_correct_key=1,
             reward_wrong_key=-1;
            bool verbose= false;
        };

        explicit BoxWorld(Option const&);

        StepResponse<cv::Mat> step(int const& action) noexcept final;

        cv::Mat reset() noexcept final;

        void render() final;
    };
}

#endif //GYMENV_BOX_WORLD_H