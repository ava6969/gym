//
// Created by dewe on 3/25/21.
//

#pragma once

#include "space.h"
#include "exception"
#include <iostream>

using std::ostream;

namespace gym::space{
    class Discrete: public Space{

    public:
        const int64_t n;

        explicit
        Discrete(int64_t _n);

        friend
        ostream& operator << (ostream& os, const Discrete& dt);

        [[nodiscard]] std::shared_ptr<Space> clone() override{
            return cloneHelper<Discrete>();
        }

        ~Discrete() override = default;

    };

    ostream& operator << (ostream& os, const Discrete& dt);
}
