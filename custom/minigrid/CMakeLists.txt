target_sources(gym PRIVATE minigrid.cpp grid.cpp object.cpp rendering.cpp wrappers.cpp)

add_subdirectory(envs)

add_executable(minigrid_ctrl ${CMAKE_CURRENT_LIST_DIR}/manual_control.cpp  )
target_link_libraries(minigrid_ctrl PRIVATE ${DEPENDENCIES} gym )
target_include_directories(minigrid_ctrl PRIVATE . ${CMAKE_CURRENT_LIST_DIR}/../..)
