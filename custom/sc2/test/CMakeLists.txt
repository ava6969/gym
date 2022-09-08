add_executable(sc2Test random_agent_test.cpp stopwatch_test.cpp protocol_error_test.cpp portpicker_test.cpp)
target_link_libraries(sc2Test PRIVATE ${DEPENDENCIES} gym )
target_include_directories(sc2Test PRIVATE . ${CMAKE_CURRENT_LIST_DIR}/../..)
