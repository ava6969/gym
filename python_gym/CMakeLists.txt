target_sources(${PROJECT_NAME} PRIVATE python_env.cpp)
find_package(pybind11 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME}  PUBLIC pybind11::embed)
install(FILES python_env.h python_env.cpp atari_wrappers.py DESTINATION include/gym/python_gym)