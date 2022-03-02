//
// Created by dewe on 8/30/21.
//

#pragma once

#include <variant>
#include "env.h"
#include "filesystem"
#include "opencv2/opencv.hpp"
#include "vector"
#include "array"
#include <unordered_set>
#include <utility>
#include "spaces/util.h"
#include "ale_c_wrapper.h"
#include "common/utils.h"


using namespace cv;
using namespace std::string_literals;

namespace gym {

    struct FrameStackVisitor{
        std::mt19937 gen;

        FrameStackVisitor():
        gen(std::mt19937(std::random_device{}()))
        {}

        int operator()(int const& frameStack){
            return frameStack;
        }

        int operator()(std::array<int, 2> const& bound){
            std::uniform_int_distribution<int> dist(bound[0], bound[1]);
            return dist(gen);
        }
    };

    template<bool image>
    using ObsT = std::conditional_t<image, cv::Mat, std::vector<char>>;

    template<bool image=true>
    class AtariEnv : public Env< ObsT<image>, int > {

    public:
        using StepT = StepResponse< ObsT<image> >;
        using ActionT = int;
        using ObservationT = ObsT<image>;

        explicit AtariEnv(std::string game,
                          std::optional<uint8_t> const &mode=std::nullopt,
                          std::optional<uint8_t> const &difficulty=std::nullopt,
                          std::variant<int, std::array<int, 2>>
                          frameSkip = 1,
                          float repeatActionProbability = 0.0f,
                          bool fullActionSpace = false, bool render=false, bool sound=false);

        void seed(std::optional<uint64_t> const& )  noexcept override;

        inline ObsT<image> reset()  noexcept override{
            this->m_Ale->reset_game();
            return getObs();
        }

        StepResponse< ObsT<image> > step(int const &action) noexcept override;

        void render(RenderType type) override;

        std::vector<std::string> getActionMeaning(){
            std::vector<std::string> actionSet(m_ActionSet.size());
            std::transform(m_ActionSet.begin(), m_ActionSet.end(), actionSet.begin(), [](auto a){
                return ACTION_MEANING[a];
            });
            return actionSet;
        }

        inline int lives() const noexcept { return this->m_Ale->lives(); }

        inline ale::ALEInterface* ale() const noexcept;

        ~AtariEnv() override{
            if(isOpened)
                cv::destroyWindow(m_Game);
        }

    private:
        std::string m_Game;
        std::string m_ObsType{};
        std::variant<int, std::array<int, 2>> m_FrameSkip{};
        std::optional<uint8_t> m_GameMode;
        std::optional<uint8_t> m_GameDifficulty;
        std::unique_ptr< ale::ALEInterface > m_Ale;
        std::vector<int> m_ActionSet;

        ObservationT data;
        bool isOpened{false};

        template<class SRC, class DEST>
        inline std::vector<DEST>  copy(std::vector<SRC> const& src) const noexcept {
            auto n = src.size();
            std::vector<DEST> dest(n);
            memcpy(dest.data(), src.data(), n*sizeof(SRC) );
            return dest;
        }

        inline
        ObsT<image> getObs() const noexcept {
            if constexpr(not image)
                getRAM(m_Ale.get(), data.data());
            else
                getScreenRGB2(m_Ale.get(), data.data);
            return data;
        }

        inline static constexpr const char* ACTION_MEANING[18] = {"NOOP", "FIRE", "UP",
                                                                  "RIGHT", "LEFT", "DOWN",
                                                                  "UPRIGHT", "UPLEFT", "DOWNRIGHT",
                                                                  "DOWNLEFT", "UPFIRE",
                                                                  "RIGHTFIRE",
                                                                  "LEFTFIRE",
                                                                  "DOWNFIRE",
                                                                  "UPRIGHTS",
                                                                  "UPLEFTFIRE",
                                                                  "DOWNRIGHTFIRE",
                                                                  "DOWNLEFTFIRE"};
    };
}
#include "atari_env.tpp"

