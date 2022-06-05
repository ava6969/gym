//
// Created by dewe on 6/4/22.
//

#include "door_key.h"

void gym::DoorKeyEnv::genGrid(int width, int height) {

    this->grid = mg::Grid(width, height);

    grid.wall_rect({0, 0, width, height});

    putObj( std::make_unique<mg::Goal>(), {width-2, height-2} );

    auto split_idx = rand<int>(2, width-2);
    grid.vert_wall(split_idx, 0);

    placeAgent( mg::Rect{0, 0, split_idx, height} );

    auto door_idx = rand<int>(1, width-2);
    putObj( std::make_unique<mg::Door>(mg::Color::Yellow, false, true), {split_idx, door_idx} );

    placeObj( std::make_unique<mg::Key>(mg::Color::Yellow), mg::Rect{0, 0, split_idx, height} );

}

gym::DoorKeyEnv::DoorKeyEnv(int size): MiniGridEnv(Option().gridSize(size).maxSteps(10*size*size)) {

    m_ActionMap = {
            Actions::left, Actions::right, Actions::forward, Actions::pickup, Actions::toggle
    };
    m_ActionSpace = makeDiscreteSpace(5);

    mission = "use the key to open the door and then get to the goal";
    std::call_once(flag, [this](){
        fillDictionary();
    });

    DoorKeyEnv::reset();

}
