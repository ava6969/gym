
IF(${ADD_OPEN_CV})
    target_sources(gym PRIVATE mountain_car.cpp)
ENDIF()
target_sources(gym PRIVATE cartpole.cpp )

install (FILES cartpole.h mountain_car.h DESTINATION include/gym/classic_control)
