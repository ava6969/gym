//
// Created by dewe on 9/25/21.
//

#ifndef GYMENV_VIZ_DOOM_H
#define GYMENV_VIZ_DOOM_H

#include "include/ViZDoom.h"
#include "unordered_map"
#include "string"
#include "../../env.h"
#include "../../common/rendering.h"
#include "filesystem"

using namespace std::string_literals;

namespace gym{
    class VizDoomEnv : public TensorEnv<VizDoomEnv> {

    private:
        bool m_Depth, m_Labels, m_Position, m_Health;
        vizdoom::DoomGame m_Game{};
        vizdoom::GameStatePtr m_State;
        std::vector<double> ZERO;
        std::array<int64_t, 3> m_Resolution{};
        std::optional<int> m_NewSize;
        TensorDict lastGoodState;

        inline static constexpr std::pair<const char*, int> CONFIGS[10] = {
                {"basic.cfg", 3},  // 0
                {"deadly_corridor.cfg", 7},  // 1
                {"defend_the_center.cfg", 3},  // 2
                {"defend_the_line.cfg", 3},  // 3
                {"health_gathering.cfg", 3},  // 4
                {"my_way_home.cfg", 5},  // 5
                {"predict_position.cfg", 3},  // 6
                {"take_cover.cfg", 2},  // 7
                {"deathmatch.cfg", 20},  // 8
                {"health_gathering_supreme.cfg", 3}  // 9
        };

        torch::Tensor resize3D(std::vector<uint8_t>& x) noexcept;

        inline auto parseScreen() noexcept{
            return m_NewSize ? resize3D(*m_State->screenBuffer) :
            torch::tensor(*m_State->screenBuffer).view(m_Resolution).permute({2, 0, 1});
        }

        TensorDict collectObservation() noexcept;

    public:

        explicit VizDoomEnv(OptionalArgMap const& arg);

        void render(RenderType type) final;

        TensorDict reset() noexcept final;

        StepResponse<TensorDict> step(torch::Tensor const& action) noexcept final;
    };
}

#endif //GYMENV_VIZ_DOOM_H
