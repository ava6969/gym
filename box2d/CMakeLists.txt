target_sources(gym PRIVATE car_dynamics.cpp car_racing.cpp contact_listen.cpp
        lunarlandar.cpp
        utils.cpp )
file(GLOB HEADERS lunarlandar.h utils.h utils.tpp )
install (FILES ${HEADERS} DESTINATION include/gym/box2d)

add_executable(car_racing control_box2d.cpp  )
target_link_libraries(car_racing PRIVATE ${DEPENDENCIES} gym )
target_include_directories(car_racing PRIVATE . ..)

add_executable(lunarlandar lunar_heuristic.cpp  )
target_link_libraries(lunarlandar PRIVATE ${DEPENDENCIES} gym )
target_include_directories(lunarlandar PRIVATE . ..)