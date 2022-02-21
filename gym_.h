//
// Created by dewe on 12/19/21.
//

#ifndef GYM_GYM_H
#define GYM_GYM_H

#include "wrappers/vec_env/vec_normalize.h"
#include "wrappers/monitor.h"
#include "wrappers/vec_env/sync_env.h"
#include "wrappers/vec_env/async_env.h"
#include "python_gym/python_env.h"
#include "classic_control/cartpole.h"

namespace gym{
    template<class VecEnvT, bool dict>
    using TrainingVecNormRO = VecNormalize<VecEnvT, dict, true, true, true>;

    template<class VecEnvT, bool dict>
    using EvalVecNormRO = VecNormalize<VecEnvT, dict, false, true, true>;

    template<class EnvT>
    using MonitorWithEarlyReset = Monitor<EnvT, true>;

}

#endif //GYM_GYM_H
