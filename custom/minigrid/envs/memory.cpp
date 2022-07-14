//
// Created by dewe on 6/4/22.
//

#include "common/utils.h"
#include "memory.h"

void gym::MemoryEnv::genGrid(int width, int height) {

    using namespace mg;
    grid = mg::Grid(width, height);

    grid.horz_wall(0, 0);
    grid.horz_wall(0, height-1);
    grid.vert_wall(0, 0);
    grid.vert_wall(width-1, 0);

    assert( height % 2 == 1);
    auto upper_room_wall = floor_div(height, 2) - 2,
    lower_room_wall = floor_div(height, 2) + 2;

    int hallway_end{};
    if(random_length)
        hallway_end = rand<int>(4, width-3);
    else
        hallway_end = width - 3;

    for (int i = 1; i < 5; ++i) {
        grid.set({i, upper_room_wall}, WorldObj::make<Wall>() );
        grid.set({i, lower_room_wall}, WorldObj::make<Wall>() );
    }
    grid.set({4, upper_room_wall+1}, WorldObj::make<Wall>() );
    grid.set({4, lower_room_wall-1}, WorldObj::make<Wall>() );

    for (int i = 4; i >= hallway_end; --i) {
        grid.set({i, upper_room_wall + 1}, WorldObj::make<Wall>() );
        grid.set({i, lower_room_wall - 1}, WorldObj::make<Wall>() );
    }

    for (int j = 0; j < height; ++j) {
        if( j != floor_div(height, 2))
            grid.set(hallway_end, j, WorldObj::make<Wall>() );
        grid.set(hallway_end+2, j, WorldObj::make<Wall>());
    }

    this->agent_pos = {rand <int>(1, hallway_end), floor_div(height, 2)};
    this->agent_dir = 0;

    using T = std::pair<int, std::function<WorldObj::Ptr(mg::Color)> >;
    auto keyBallMaker = std::vector< T >{
            std::pair(0, [](mg::Color c) {
                return WorldObj::make<Key>(c);
            }),
            std::pair(1, [](mg::Color c) {
                return WorldObj::make<Ball>(c);
            })
    };

    auto ballKeyMaker = std::vector< T >{
            std::pair(1, [](mg::Color c) {
                return WorldObj::make<Ball>(c);
            }),
            std::pair(0, [](mg::Color c) {
                return WorldObj::make<Key>(c);
            })
    };

    auto start_room_obj = rand( keyBallMaker );
    grid.set(1, floor_div(height, 2) - 1, start_room_obj.second(mg::Color::Green));

    std::vector< std::vector<T> > cont{ballKeyMaker, keyBallMaker};

    auto other_objs = rand(cont);
    auto pos0 = mg::Point{hallway_end+1, floor_div(height, 2) - 2},
         pos1 = mg::Point{hallway_end+1, floor_div(height, 2) + 2};

    grid.set(pos0, other_objs[0].second(mg::Color::Green));
    grid.set(pos1, other_objs[1].second(mg::Color::Green));

    if( start_room_obj.first == other_objs[0].first){
        success_pos = {pos0.x, pos0.y + 1};
        failure_pos = {pos1.x, pos1.y - 1};
    }else{
        success_pos = {pos1.x, pos1.y - 1};
        failure_pos = {pos0.x, pos0.y + 1};
    }

}

gym::StepResponse< std::unordered_map< std::string, cv::Mat> > gym::MemoryEnv::step(const int &_action) noexcept {
    auto action = _action;
    if( action == mg::toIndex(Actions::pickup ) )
        action = mg::toIndex(Actions::toggle );

    auto response = MiniGridEnv::step(action);

    if( agent_pos.value() == success_pos){
        response.reward = this->reward();
        response.done = true;
    }
    if( *agent_pos == failure_pos){
        response.reward = 0;
        response.done = true;
    }

    return response;
}

gym::MemoryEnv::MemoryEnv(std::optional<int>  seed, int size, bool random_length):
        random_length( random_length ),
        MiniGridEnv( Option(seed).gridSize(size).maxSteps(5*size*size).seeThroughWalls(false) ){

    mission = "go to the matching object at the end of the hallway'";

    m_ActionMap = {
            Actions::left, Actions::right, Actions::forward
    };
    m_ActionSpace = makeDiscreteSpace(3);

    std::call_once(flag, [this](){
        fillDictionary();
    });

    MemoryEnv::reset();

}
