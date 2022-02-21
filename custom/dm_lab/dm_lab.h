//
// Created by dewe on 9/2/21.
//

#ifndef GYMENV_DM_LAB_H
#define GYMENV_DM_LAB_H

#include "lab.h"
#include "env.h"
#include "spaces/dict.h"
#include "action_mapping.h"
#include <utility>

namespace gym {

    class DMLabEnv : public TensorEnv<DMLabEnv>{

    private:
        int m_numSteps;
        Lab m_Lab;
        std::vector<std::map<string, int>> m_ActionSet;

    public:
        DMLabEnv(OptionalArgMap const& args);

        TensorDict reset() noexcept final;

        StepResponse<TensorDict> step(torch::Tensor const& action) noexcept final;

        void render(RenderType type) final;
    };
}

#endif //GYMENV_DM_LAB_H
