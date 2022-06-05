#pragma once
//
// Created by dewe on 6/4/22.
//

#include "../minigrid.h"
namespace gym{

    struct MemoryEnv : MiniGridEnv{

        explicit MemoryEnv( std::optional<int> seed, int size=8, bool random_length=false);

        void genGrid(int width, int height) override;

        StepT step(const ActionT &action) noexcept override;

    private:
        bool random_length;
        mg::Point success_pos{}, failure_pos{};
        static inline std::once_flag flag{};
    };

    struct MemoryS17Random : MemoryEnv{
        explicit MemoryS17Random():MemoryEnv( std::nullopt, 17, true ){};
    };

    struct MemoryS13Random : MemoryEnv{
        explicit MemoryS13Random():MemoryEnv( std::nullopt, 13, true ){};
    };

    struct MemoryS13 : MemoryEnv{
        explicit MemoryS13():MemoryEnv( std::nullopt, 13 ){};
    };

    struct MemoryS9 : MemoryEnv{
        explicit MemoryS9():MemoryEnv( std::nullopt, 9 ){};
    };

    struct MemoryS7 : MemoryEnv{
        explicit MemoryS7():MemoryEnv( std::nullopt, 7 ){};
    };
}