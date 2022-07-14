//
// Created by dewe on 6/29/22.
//

#include "speed_test.h"
#include "classic_control/cartpole.h"
#include "classic_control/mountain_car.h"
#include "box2d/lunarlandar.h"
#include "box2d/car_racing.h"
#include "atari/atari_env.h"
#include "custom/box_world/box_world.h"
#include "custom/box_world/random_box_world.h"
#include "custom/dm_lab/dm_lab.h"
#include "custom/minigrid/envs/door_key.h"
#include "custom/minigrid/envs/memory.h"
#include "custom/procgen/procgen.h"

using namespace gym;

int main(){
//    speed_test<DMLabEnv>(1, discreteRandomPolicy<DMLabEnv>, "DMLabEnv::language_select_described_object", DMLabEnv::Option("language_select_described_object") );
    speed_test<ProcgenEnv>(10, discreteRandomPolicy<ProcgenEnv>, "ProcgenEnv::Coinrun",
            BaseProcgenEnv::Option("coinrun").resourceRoot("/home/dewe/sam/gym/custom/procgen/data/assets/") );
    speed_test<ProcgenEnv>(10, discreteRandomPolicy<ProcgenEnv>, "ProcgenEnv::Bigfish",
            BaseProcgenEnv::Option("bigfish").resourceRoot("/home/dewe/sam/gym/custom/procgen/data/assets/") );
    speed_test<DoorKeyEnv5x5>(10, discreteRandomPolicy<DoorKeyEnv5x5>, "DoorKeyEnv5x5");
    speed_test<DoorKeyEnv16x16>(10, discreteRandomPolicy<DoorKeyEnv16x16>, "DoorKeyEnv16x16");
    speed_test<MemoryS13>(10, discreteRandomPolicy<MemoryS13>, "MemoryS13");
    speed_test<MemoryS7>(10, discreteRandomPolicy<MemoryS7>, "MemoryS7");
    speed_test<DMLabEnv>(1, discreteRandomPolicy<DMLabEnv>, "DMLabEnv::lt_chasm", DMLabEnv::Option() );
    speed_test<DMLabEnv>(1, discreteRandomPolicy<DMLabEnv>, "DMLabEnv::rooms_watermaze", DMLabEnv::Option("rooms_watermaze") );
    speed_test<CartPoleEnv, DiscretePolicyT<CartPoleEnv>>(1);
    speed_test<MountainCarEnv, DiscretePolicyT<MountainCarEnv>>(1);
    speed_test<LunarLandarEnv<false>, DiscretePolicyT<LunarLandarEnv<false>>>(10);
    speed_test<LunarLandarEnv<true>>(10, multiContinuousRandomPolicy<LunarLandarEnv<true>>);
    speed_test<BoxWorld>(10, discreteRandomPolicy<BoxWorld>, std::nullopt, BoxWorld::Option());
    speed_test<RandomBoxWorld>(10, discreteRandomPolicy<RandomBoxWorld>, std::nullopt, RandomBoxWorld::Option() );
    speed_test<AtariEnv<true>>(10, discreteRandomPolicy<AtariEnv<true>>, "pong_image", "pong" );
    speed_test<AtariEnv<true>>(10, discreteRandomPolicy<AtariEnv<true>>, "bowling_image", "bowling" );
    speed_test<AtariEnv<false>>(10, discreteRandomPolicy<AtariEnv<false>>, "alien_ram", "alien" );
    speed_test<AtariEnv<false>>(10, discreteRandomPolicy<AtariEnv<false>>, "air_raid_ram", "air_raid" );
    speed_test<CarRacing>(10, multiContinuousRandomPolicy<CarRacing>, "CarRacing", true, false );

    return 0;
}