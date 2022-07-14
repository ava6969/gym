#pragma once
//
// Created by dewe on 6/30/22.
//

#include "randomkit.h"
#include "vector"
#include "optional"

namespace np{


    typedef double (* rk_cont0)(rk_state *state);
    typedef double (* rk_cont1)(rk_state *state, double a);
    typedef double (* rk_cont2)(rk_state *state, double a, double b);
    typedef double (* rk_cont3)(rk_state *state, double a, double b, double c);
    typedef long (* rk_disc0)(rk_state *state);
    typedef long (* rk_discnp)(rk_state *state, long n, double p);
    typedef long (* rk_discdd)(rk_state *state, double n, double p);
    typedef long (* rk_discnmN)(rk_state *state, long n, long m, long N);
    typedef long (* rk_discd)(rk_state *state, double a) ;

    static std::vector<double> cont2_array_sc(rk_state& state, rk_cont2 const& func, size_t size, double a, double b){
        std::vector<double> array(size);
        std::generate(std::begin(array), std::end(array), [&func, &state, a, b]{ return func(&state, a, b); });
        return array;
    }

    static double cont2_array_sc(rk_state& state, rk_cont2 const& func, double a, double b){
        return func(&state, a, b);
    }

}