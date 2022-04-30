//
// Created by dewe on 8/31/21.
//

#include "time_limit.h"
#include "atari_wrappers.h"

namespace gym{

    StepResponse <ObsT<true>> MaxAndSkipEnv::step(const int &action)  noexcept   {

        float totalReward = 0.0;
        StepT resp;

        int t = 0;
        while (t < m_FrameSkip){
            resp = this->m_Env->step(action);

            if( t == m_FrameSkip - 2){
                m_ObsBuffer[0] = std::move(resp.observation);
            }else if ( t == m_FrameSkip - 1){
                m_ObsBuffer[1] = std::move(resp.observation);
            }
            totalReward += resp.reward;
            if(resp.done)
                break;
            t++;
        }

        resp.observation = cv::max(m_ObsBuffer[0], m_ObsBuffer[1]);
        resp.reward = totalReward;
        return resp;
    }

    StepResponse < ObsT<true> > EpisodicLifeEnv::step(const int &action) noexcept {
        auto resp = m_Env->step(action);

        m_WasRealDone = resp.done;

        int lives =  std::any_cast<int>(resp.info["ale.lives"]);

        if( 0 < lives and lives < m_Lives){
            resp.done = true;
        }
        m_Lives = lives;
        return resp;
    }

    ObsT<true> EpisodicLifeEnv::reset() noexcept {
        auto output = m_WasRealDone ? m_Env->reset() : m_Env->step(0).observation;
        m_Lives = parent->lives();
        return output;
    }

    ObsT<true> FireResetEnv::reset()  noexcept {
        m_Env->reset();

        auto resp = m_Env->step(1);
        if(resp.done)
            m_Env->reset();

        resp = m_Env->step(2);
        if(resp.done)
            m_Env->reset();

        return resp.observation;
    }

    std::unique_ptr< Env<ObsT<true>, int> > AtariWrapper::make( std::unique_ptr< Env<ObsT<true>, int>>  env,
                                                                  int noopMax,
                                                                  int frameSkip,
                                                                  int screenSize,
                                                                  bool terminalOnLifeLoss,
                                                                  bool clipReward){
        auto* baseAtariEnv = dynamic_cast<AtariEnv<true>*>(env.get());
        std::vector<std::string> action_meaning;
        bool update_parent=false;

        if( not baseAtariEnv){
            auto* wrapped = dynamic_cast<gym::Wrapper< Env<ObsT<true>, int> >*>(env.get());
            if (wrapped)
                baseAtariEnv = wrapped->try_cast<AtariEnv<true>>();
            else{
                if( auto time_limit = dynamic_cast< gym::TimeLimit< AtariEnv<true> > * >(env.get()) ){
                    baseAtariEnv = time_limit->try_cast<AtariEnv<true>>();
                    update_parent = true;
                }
            }

            if(not baseAtariEnv){
                throw std::runtime_error("Gym Env passed into AtariWrapper has no pointer or relationship to an Atari");
            }
        }

        action_meaning = baseAtariEnv->getActionMeaning();
        std::unique_ptr< Env<ObsT<true>, int>> wrappedEnv = std::make_unique<NOOPResetEnv>(move(env),
                                                                                           action_meaning,
                                                                                           noopMax);
        wrappedEnv = std::make_unique<MaxAndSkipEnv>(move(wrappedEnv), frameSkip);

        wrappedEnv = std::make_unique< Monitor< Env<ObsT<true>, int>, true> >( std::move(wrappedEnv) );

        if(terminalOnLifeLoss){
            if(update_parent){
                wrappedEnv = std::make_unique<EpisodicLifeEnv>(move(wrappedEnv), baseAtariEnv);
            }else{
                wrappedEnv = std::make_unique<EpisodicLifeEnv>(move(wrappedEnv));
            }
        }

        if(std::find(action_meaning.begin(), action_meaning.end(), "FIRE") != action_meaning.end()){
            wrappedEnv = std::make_unique<FireResetEnv>(move(wrappedEnv));
        }

        wrappedEnv = std::make_unique<WarpFrameEnv>(move(wrappedEnv), screenSize, screenSize);

        if(clipReward){
            wrappedEnv = std::make_unique<ClipRewardEnv< Env<ObsT<true>, int> >>(move(wrappedEnv));
        }

        return std::make_unique<AtariWrapper>( std::move(wrappedEnv) );
    }
}
