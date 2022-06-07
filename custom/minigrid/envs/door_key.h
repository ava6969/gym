#pragma once
//
// Created by dewe on 6/4/22.
//

#include "../minigrid.h"
namespace gym{

    struct DoorKeyEnv : MiniGridEnv{

        explicit DoorKeyEnv(int size);

        void genGrid(int width, int height) override;

    private:
        static inline std::once_flag flag{};
    };

    struct DoorKeyEnv5x5 : DoorKeyEnv{
        explicit DoorKeyEnv5x5(): DoorKeyEnv(5){}
    };

    struct DoorKeyEnv6x6 : DoorKeyEnv{
        explicit DoorKeyEnv6x6(): DoorKeyEnv(6){}
    };

    struct DoorKeyEnv8x8 : DoorKeyEnv{
        explicit DoorKeyEnv8x8(): DoorKeyEnv(8){}
    };

    struct DoorKeyEnv16x16 : DoorKeyEnv{
        explicit DoorKeyEnv16x16(): DoorKeyEnv(16){}
    };
}