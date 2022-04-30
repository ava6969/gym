//
// Created by dewe on 9/1/21.
//

#ifndef GYMENV_MAX_SKIP_H
#define GYMENV_MAX_SKIP_H

#include "wrappers/atari_wrappers.h"

namespace gym {

    class MaxAndSkipEnv : public Wrapper< Env<ObsT<true>, int> > {

    private:
        std::array<cv::Mat, 2> m_ObsBuffer;
        int m_FrameSkip{};

    public:
        TYPENAME_INFO
        DEFAULT_RESET_OVERRIDE
        DEFAULT_STEP_OVERRIDE
        DEFAULT_RESET_OVERRIDES

        explicit MaxAndSkipEnv(std::unique_ptr<Env< ObservationT, int>> env, int skip=4):
                Wrapper(std::move(env)),
                m_FrameSkip(skip){}

        StepResponse< ObsT<true> > step(const int &action) noexcept override;
    };
}

#endif //GYMENV_MAX_SKIP_H
