//
// Created by dewe on 9/1/21.
//

#ifndef GYMENV_WARP_FRAME_H
#define GYMENV_WARP_FRAME_H

#include "common/wrapper.h"
//#include "atari/atari_env.h"


namespace gym {

    class WarpFrameEnv : public ObservationWrapper< ObsT<true>, int> {

    private:
        int m_Width{}, m_Height{};
    public:

        explicit
        WarpFrameEnv(std::unique_ptr<Env<ObsT<true>, int>> env,
                     int width,
                     int height):
                     ObservationWrapper<ObsT<true>, int>(std::move(env)),
                m_Width(width),
                m_Height(height){

                this->m_ObservationSpace = makeBoxSpace<uint8_t>(0, 255, {m_Height, m_Width, 1});

        }

        explicit
        WarpFrameEnv(std::shared_ptr<Space> obsSpace, std::shared_ptr<Space> actSpace,
                     int width, int height):ObservationWrapper(obsSpace, actSpace),
                                 m_Width(width),
                                 m_Height(height){

            this->m_ObservationSpace = makeBoxSpace<uint8_t>(0, 255, {m_Height, m_Width, 1});
        }

    [[nodiscard]] ObsT<true> observation(ObsT<true>&& raw_img) const noexcept final {
            cv::cvtColor(raw_img, raw_img, cv::COLOR_RGB2GRAY);
            cv::resize(raw_img, raw_img,
                       {m_Width, m_Height},
                       0, 0, INTER_AREA);
            return std::move(raw_img);
        }
    };
}


#endif //GYMENV_WARP_FRAME_H
