add_executable(speed_test speed_test.cpp)
target_link_libraries(speed_test PRIVATE ${DEPENDENCIES} gym)
target_include_directories(speed_test PRIVATE ..)