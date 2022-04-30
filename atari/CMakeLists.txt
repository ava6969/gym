include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(ale)

find_package(SDL2 CONFIG REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ale)
target_link_libraries(${PROJECT_NAME} PUBLIC SDL2::SDL2)
target_sources(${PROJECT_NAME} PRIVATE atari_env.cpp ale_c_wrapper.cpp)