//
// Created by dewe on 4/15/21.
//

#include <random>
#include "discrete.h"

namespace gym::space{
    ostream &operator<<(ostream &os, const Discrete &dt) {

        os << "Discrete(" << dt.n << ")";
        return os;
    }

    Discrete::Discrete(int64_t _n):
    Space(Shape{{_n}, c10::kInt}),
    n(_n) {

        if (_n <= 0)
            throw std::invalid_argument("discrete space only accepts dim > 0");
    }
}
