//
// Created by dewe on 8/31/21.
//

#ifndef GYMENV_ATARI_WRAPPERS_H
#define GYMENV_ATARI_WRAPPERS_H

#include <utility>
#include "common/wrapper.h"
#include "type_traits"
#include "noop_reset.h"
#include "clip_action.h"
#include "episodic_life.h"
#include "fire_reset.h"
#include "max_skip.h"
#include "warp_frame.h"
#include "clip_reward.h"
#include "gym_.h"
#include "wrappers/vec_env/vec_atari.h"

namespace gym {

    class AtariWrapper : public Wrapper< Env<ObsT<true>, int> > {

    public:

        explicit AtariWrapper(std::unique_ptr<Env<ObsT<true>, int>> env):Wrapper< Env<ObsT<true>, int> >( std::move(env) ){}

        explicit AtariWrapper(std::unique_ptr<Env<ObsT<true>, int>> env,
                              int noopMax,
                              int frameSkip,
                              int screenSize,
                              bool terminalOnLifeLoss,
                              bool clipReward):
                Wrapper< Env<ObsT<true>, int> >( std::move(make(std::move(env),
                                                         noopMax,
                                                         frameSkip,
                                                         screenSize,
                                                         terminalOnLifeLoss,
                                                         clipReward)) ) {}

        inline typename AtariWrapper::StepT step(const int & action) noexcept override{
            auto resp = this->m_Env->step(action);
            lastObs = resp.observation;
            return resp;
        }

        inline ObsT<true> reset() noexcept override{
            lastObs = m_Env->reset();
            return lastObs;
        }

        void render(RenderType type) override{
            cv::imshow("Atari PreProc", lastObs);
            cv::waitKey(1);
        }

        static
        std::unique_ptr< Env<ObsT<true>, int> > make( std::unique_ptr< Env<ObsT<true>, int>>  env,
                                                      int noopMax = 30,
                                                      int frameSkip = 4,
                                                      int screenSize = 84,
                                                      bool terminalOnLifeLoss = true,
                                                      bool clipReward = true);

    private:
        ObsT<true> lastObs;

    };
}

#endif //GYMENV_ATARI_WRAPPERS_H
