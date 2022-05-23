add_executable(dm_lab_run_test  dm_lab.cpp )
target_link_libraries(dm_lab_run_test PRIVATE ${DEPENDENCIES} gym)
target_include_directories(dm_lab_run_test PRIVATE ..)

add_executable(GymTest  box_world.cpp py_atari_env.cpp procgen.cpp)
target_link_libraries(GymTest PRIVATE ${DEPENDENCIES} gym)
target_include_directories(GymTest PRIVATE ..)

if(${ADD_PROCGEN})
    # include qt5
    set(CMAKE_PREFIX_PATH ${SAM_PATH}/thirdparty/qt)
    find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)
    target_link_libraries(GymTest PUBLIC Qt5::Core Qt5::Widgets Qt5::Gui)
ENDIF()
