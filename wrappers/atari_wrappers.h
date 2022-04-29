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

//    template<bool TerminalOnLossAndCanFire>
//    using AtariWrapperT = std::conditional_t<TerminalOnLossAndCanFire,
//            WrapperUnroll< FireResetEnv,  WarpFrameEnv,  ClipRewardEnv>,
//            WrapperUnroll< MaxAndSkipEnv,  WarpFrameEnv,  ClipRewardEnv>>;

    class AtariWrapper : public Wrapper<ObsT<true>, int> {

    public:

        using ObservationT = ObsT<true>;
        using ActionT = int;
        using StepT = StepResponse <ObsT<true>>;

        explicit AtariWrapper(std::unique_ptr<Env<ObsT<true>, int>> env,
                              int noopMax = 30,
                              int frameSkip = 4,
                              int screenSize = 84,
                              bool terminalOnLifeLoss = true,
                              bool clipReward = true):
                Wrapper< ObsT<true>, int>(std::move(make(std::move(env),
                                                         noopMax,
                                                         frameSkip,
                                                         screenSize,
                                                         terminalOnLifeLoss,
                                                         clipReward))) {}

        inline StepT step(const int & action) noexcept override{
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
        std::unique_ptr<gym::Wrapper<ObsT<true>, int>> make(std::unique_ptr<Env<ObsT<true>, int>>  env,
                                                     int noopMax,
                                                     int frameSkip,
                                                     int screenSize,
                                                     bool terminalOnLifeLoss,
                                                     bool clipReward){
            auto* ENV_ = dynamic_cast<AtariEnv<>*>(env.get());
            std::vector<std::string> meaning;

            if( not ENV_){
                auto* wrapped = dynamic_cast<gym::Wrapper<ObsT<true>, int>*>(env.get());
                assert(wrapped);
                ENV_ = wrapped->try_cast<AtariEnv<>>();
            }

            meaning = ENV_->getActionMeaning();
            std::unique_ptr<Wrapper <ObsT<true>, int> > wrappedEnv = std::make_unique<NOOPResetEnv>(move(env),
                                                                                                    meaning,
                                                                                                    noopMax);
            wrappedEnv = std::make_unique<MaxAndSkipEnv>(move(wrappedEnv), frameSkip);

            wrappedEnv = MonitorWithEarlyReset< MaxAndSkipEnv>>(
                    std::move(wrappedEnv),
                    std::numeric_limits<double>::max());

            if(terminalOnLifeLoss)
                wrappedEnv = std::make_unique<EpisodicLifeEnv>(move(wrappedEnv));

            if(std::find(meaning.begin(), meaning.end(), "FIRE") != meaning.end()){
                wrappedEnv = std::make_unique<FireResetEnv>(move(wrappedEnv));
            }

            wrappedEnv = std::make_unique<WarpFrameEnv>(move(wrappedEnv), screenSize, screenSize);

            if(clipReward){
                wrappedEnv = std::make_unique<ClipRewardEnv<true>>(move(wrappedEnv));
            }

            return wrappedEnv;
        }

//        template<bool TerminalOnLoss, bool CanFire>
//        static AtariWrapperT<TerminalOnLoss and CanFire> makeAtariWrapperT(std::string game, int noopMax = 30,
//                              int frameSkip = 4, int screenSize = 84, bool clipReward = true){
//
//            auto env = std::make_unique< AtariEnv<true> >( std::move(game) );
//            std::unique_ptr<Wrapper> wrappedEnv = std::make_unique<NOOPResetEnv>(move(env), noopMax);
//            wrappedEnv = std::make_unique<MaxAndSkipEnv>(move(wrappedEnv), frameSkip);
//            std::shared_ptr<Space> o = wrappedEnv->observationSpace();
//            std::shared_ptr<Space> a = wrappedEnv->actionSpace();
//
//            if constexpr ( TerminalOnLoss and CanFire){
//                wrappedEnv = std::make_unique<EpisodicLifeEnv>(move(wrappedEnv));
//                return    WrapperUnroll< FireResetEnv,  WarpFrameEnv,  ClipRewardEnv,  VecNormAndPermute>{
//                    FireResetEnv(std::move(wrappedEnv)),
//                    WarpFrameEnv(o, a, screenSize, screenSize),
//                    ClipRewardEnv(o, a),
//                    VecNormAndPermute(o, a)};
//            }else{
//                return  WrapperUnroll< MaxAndSkipEnv,  WarpFrameEnv,  ClipRewardEnv,  VecNormAndPermute>(
//                    std::move( *dynamic_cast<MaxAndSkipEnv*>(wrappedEnv.get()) ),
//                         WarpFrameEnv(o, a, screenSize, screenSize),
//                         ClipRewardEnv(o, a),
//                         VecNormAndPermute(o, a));
//            }
//        }

    private:
        ObsT<true> lastObs;
    };


}

#endif //GYMENV_ATARI_WRAPPERS_H
