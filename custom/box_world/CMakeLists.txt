target_sources(gym PRIVATE box_world.cpp random_box_world.cpp)

install (FILES box_world.h DESTINATION include/gym/custom/box_world)