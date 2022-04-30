//
// Created by dewe on 9/1/21.
//

#ifndef GYMENV_NOOP_RESET_H
#define GYMENV_NOOP_RESET_H

#include "common/wrapper.h"
#include "atari/atari_env.h"

namespace gym {

    class NOOPResetEnv : public Wrapper< Env< ObsT<true>, int> > {

    private:
        std::optional<int> m_OverrideNumNoops{std::nullopt};
        const int m_NoopAction;
        const int m_NoopMax;

    public:
        DEFAULT_RESET_OVERRIDE
        DEFAULT_STEP_OVERRIDE

        explicit NOOPResetEnv(std::unique_ptr<Env<ObsT<true>, int>> env,
                              int noopMax=30):
                              Wrapper(std::move(env)),
                                      m_NoopAction(0),
                                      m_NoopMax(noopMax){
            auto meaning = dynamic_cast<AtariEnv<true>*>(this->m_Env.get())->getActionMeaning();
            auto firstActionIsNOOP = std::find(meaning.begin(), meaning.end(), "NOOP") != meaning.end();
            assert(firstActionIsNOOP);
        }

        explicit NOOPResetEnv(std::unique_ptr<Env<ObsT<true>, int>> env, std::vector<std::string> const& meaning,
                              int noopMax=30):
                Wrapper(std::move(env)),
                m_NoopAction(0),
                m_NoopMax(noopMax){
            auto firstActionIsNOOP = std::find(meaning.begin(), meaning.end(), "NOOP") != meaning.end();
            assert(firstActionIsNOOP);
        }

        ObservationT reset() noexcept override {
            StepT resp;
            resp.observation = this->m_Env->reset();

            int noops = m_OverrideNumNoops.value_or(
                    std::uniform_int_distribution<int>(1, m_NoopMax + 1)(this->m_Device));

            assert(noops > 0);
            // obs = np.zeros(0) ignore since observation is replaced anyway
            while (noops-- > 0){
                resp = this->m_Env->step(m_NoopAction);
                if(resp.done)
                    resp.observation = this->m_Env->reset();
            }
            return resp.observation;
        }

    };
}


#endif //GYMENV_NOOP_RESET_H
