target_sources(gym PRIVATE box_world.cpp random_box_world.cpp)

install (FILES box_world.h DESTINATION include/gym/custom/box_world)

add_executable(box_world manual_control.cpp  )
target_link_libraries(box_world PRIVATE ${DEPENDENCIES} gym )
target_include_directories(box_world PRIVATE . ${CMAKE_CURRENT_LIST_DIR}/../..)