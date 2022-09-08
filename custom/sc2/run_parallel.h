#pragma once
//
// Created by dewe on 7/14/22.
//

#include "optional"
#include "future"
#include "lib/util.h"


class RunParallel {

public:
    explicit RunParallel(std::optional<int> const& timeout=std::nullopt){}

    template<class Callable>
    std::vector<ReturnType<Callable>> run(std::vector<Callable> const& funcs){
        auto N = funcs.size();
        using RetT = ReturnType<Callable>;

        std::vector<RetT> result(N);

        if( N == 1){
            return { funcs[0]() };
        }
        if( N > m_workers){
//            shutdown();
            m_workers = funcs.size();
        }

        std::vector<std::future<RetT> > futs(N);
        std::ranges::transform(funcs, futs.begin(), [](auto const& f){
            return std::async(f);
        });

        std::ranges::transform(futs, result.begin(), [](auto const& f){
            return f.get();
        });

        return result;

    }

private:
    std::optional<int> m_timeout ={};
    int m_workers{};



};
