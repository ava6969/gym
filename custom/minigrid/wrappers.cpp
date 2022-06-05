//
// Created by dewe on 6/5/22.
//

#include "wrappers.h"

namespace gym{

    ViewSizeWrapper::ViewSizeWrapper( std::unique_ptr<MiniGridEnv> env, int agent_view_size ):
    ObservationWrapper< MiniGridEnv  >( std::move(env) ){

        assert( agent_view_size % 2 == 1);
        assert( agent_view_size >= 3);

        m_Env->setAgentSize( agent_view_size );
        m_ObservationSpace = makeBoxSpace<uint8_t>( 0, 255, {agent_view_size, agent_view_size, 3});
    }
}