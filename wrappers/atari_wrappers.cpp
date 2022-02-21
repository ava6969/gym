//
// Created by dewe on 8/31/21.
//

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
}
