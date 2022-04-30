
target_sources(gym PRIVATE mountain_car.cpp cartpole.cpp )
install (FILES cartpole.h mountain_car.h DESTINATION include/gym/classic_control)
