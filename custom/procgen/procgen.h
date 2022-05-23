//
// Created by dewe on 5/21/22.
//

#pragma once

#include "string"
#include "unordered_map"
#include "cmath"
#include "opencv2/opencv.hpp"
#include "custom/procgen/src/vecoptions.h"
#include "env.h"

namespace gym {


    static constexpr size_t MAX_STATE_SIZE = 2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2;

    static constexpr auto ENV_NAMES = {
            "bigfish",
            "bossfight",
            "caveflyer",
            "chaser",
            "climber",
            "coinrun",
            "dodgeball",
            "fruitbot",
            "heist",
            "jumper",
            "leaper",
            "maze",
            "miner",
            "ninja",
            "plunder",
            "starpilot"
    };

    static const std::unordered_map<std::string, uint64_t> EXPLORATION_LEVEL_SEEDS = {
            {"coinrun", 1949448038},
            {"caveflyer", 1259048185},
            {"leaper", 1318677581},
            {"jumper", 1434825276},
            {"maze", 158988835},
            {"heist", 876640971},
            {"climber", 1561126160},
            {"ninja", 1123500215},
    };

    static const std::unordered_map<std::string, uint64_t> DISTRIBUTION_MODE_DICT = {
            {"easy", 0},
            {"hard", 1},
            {"extreme", 2},
            {"memory", 10},
            {"exploration", 20}
    };

    class BaseProcgenEnv {

    public:

        struct Option{
            std::string env_name;
            int num_levels=0, start_level=0, num_threads=0, num_actions=0, debug_mode=0;
            bool use_sequential_levels=false;

            int rand_seed = -1;
            bool render_human;
            std::string resource_root;
            bool center_agent=true,
            use_backgrounds=true,
            use_monochrome_assets=false,
            restrict_themes=false,
            use_generated_assets=false,
            paint_vel_info=false;
            std::string distribution_mode="hard";
            int distribution_mode_enum=0;
        };

        explicit BaseProcgenEnv(Option const& opt);

        std::array<char, MAX_STATE_SIZE> getState();
        void setState(std::array<char, MAX_STATE_SIZE> && );

        static inline std::vector<std::array<std::string, 2>> getCombos(){
            return {{"LEFT", "DOWN"},
                    {"LEFT"},
                    {"LEFT", "UP"},
                    {"DOWN"},
                    {},
                    {"UP"},
                    {"RIGHT", "DOWN"},
                    {"RIGHT"},
                    {"RIGHT", "UP"},
                    {"D"},
                    {"A"},
                    {"W"},
                    {"S"},
                    {"Q"},
                    {"E"}};
        }

        void act( int32_t const& ac);

        std::tuple<cv::Mat, float, bool> observe( );

        cv::Mat reset();

        ~BaseProcgenEnv();

    private:
        libenv_options makeOption(Option const& opt);
        std::array<char, MAX_STATE_SIZE> state{};

        BaseProcgenEnv::Option base_opt;
        std::vector<libenv_option> opts;
        std::vector<std::array<std::string, 2>> combos;
        libenv_env* handle;

        std::shared_ptr< class Game > game;
        int32_t** action_ref{};
        uint8_t** obs_ref{};
        uint32_t ** prev_level_seed{};
        uint8_t ** prev_level_complete{}, rgb{};
        uint32_t ** level_seed{};
        cv::Mat obs_data;
        std::array<float, 1>  reward_ref{};
        std::array<bool, 1>  first_ref{};
    };


class ProcgenEnv : public Env<cv::Mat, int>{
        BaseProcgenEnv base;
        std::optional<cv::Mat> resetData;


    public:
        explicit ProcgenEnv(BaseProcgenEnv::Option const& option);

        ObservationT reset() noexcept override;

        StepT step(const ActionT &action) noexcept override;

        void render(RenderType) override;
    };
}