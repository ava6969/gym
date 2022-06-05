#pragma once
//
// Created by dewe on 6/5/22.
//

#include "common/wrapper.h"
#include "minigrid.h"

namespace gym{

    struct ViewSizeWrapper : ObservationWrapper< MiniGridEnv  >{

        ViewSizeWrapper( std::unique_ptr<MiniGridEnv> env, int agent_view_size=7 );

    };
}