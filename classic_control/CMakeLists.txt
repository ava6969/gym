target_sources(gym PRIVATE cartpole.cpp mountain_car.cpp)

install (FILES cartpole.h mountain_car.h DESTINATION include/gym/classic_control)
