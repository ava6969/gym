#pragma once
//
// Created by dewe on 6/18/22.
//

#include "common/wrapper.h"
#include "common/utils.h"

namespace gym{

    template<class EnvT>
    struct ContinuousSampling : ActionWrapper2<EnvT, int>{

        ContinuousSampling(std::shared_ptr< EnvT > env,
                      std::vector<std::vector<float>> && action_mapping):
                      ActionWrapper2<EnvT, int>(makeDiscreteSpace(action_mapping.size()), std::move(env)),
                      actionMapping(action_mapping){}

      inline std::vector<float> action( int const& a) const noexcept override{
          return actionMapping.at( a );
     }

    private:
        std::vector<std::vector<float>> actionMapping;

    };
}