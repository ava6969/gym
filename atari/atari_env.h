//
// Created by dewe on 8/30/21.
//

#pragma once

#include <variant>
#include "env.h"
#include "filesystem"
#include "vector"
#include "array"
#include <unordered_set>
#include <utility>
#include "spaces/util.h"
#include "common/utils.h"
#include "ale_c_wrapper.h"

using namespace std::string_literals;

namespace gym {

    template<bool image>
    using ObsT = std::conditional_t<image, cv::Mat, std::vector<uint8_t>>;

    template<bool image=true>
    class AtariEnv : public Env< ObsT<image>, int > {

    public:

        explicit AtariEnv(std::string game,
                          std::optional<uint8_t> const &mode=std::nullopt,
                          std::optional<uint8_t> const &difficulty=std::nullopt,
                          std::variant<int, std::array<int, 2>>
                          frameSkip = 1,
                          float repeatActionProbability = 0.0f,
                          bool fullActionSpace = false, bool render=false, bool sound=false);

        void seed(std::optional<uint64_t> const& )  noexcept override;

        ObsT<image> reset()  noexcept override;

        StepResponse< ObsT<image> > step(int const &action) noexcept override;

        void render() override;

        std::vector<std::string> getActionMeaning();

        [[nodiscard]] int lives() const noexcept;

        [[nodiscard]] ale::ALEInterface* ale() const noexcept;

        ~AtariEnv() override;

    private:
        std::string m_Game;
        std::string m_ObsType{};
        std::variant<int, std::array<int, 2>> m_FrameSkip{};
        std::optional<uint8_t> m_GameMode;
        std::optional<uint8_t> m_GameDifficulty;
        std::unique_ptr< ale::ALEInterface > m_Ale;
        std::vector<int> m_ActionSet;

        ObsT<image> data;
        bool isOpened{false};

        template<class SRC, class DEST>
        std::vector<DEST>  copy(std::vector<SRC> const& src) const noexcept;

        ObsT<image> getObs() noexcept;

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

    extern template class AtariEnv<true>;
    extern template class AtariEnv<false>;
}



