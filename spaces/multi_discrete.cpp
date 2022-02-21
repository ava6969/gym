//
// Created by dewe on 8/22/21.
//

#include "multi_discrete.h"

namespace gym::space {
    ostream &operator<<(ostream &os, const MultiDiscrete &dt) {

        os << "MultiDiscrete(" << stringifyVector(dt.m_Dims) << ")";
        return os;
    }
}