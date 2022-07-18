#pragma once
//
// Created by dewe on 7/16/22.
//

#include "../maps/lib.h"


namespace sc2{
    class RemoteController {

    public:
        std::vector< std::shared_ptr<Map> > available_maps();
    };
}
