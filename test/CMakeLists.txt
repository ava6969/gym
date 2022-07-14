add_executable(GymTest  box_world.cpp py_atari_env.cpp procgen.cpp box2d.cpp dm_lab.cpp random.cpp classic_control.cpp)
target_link_libraries(GymTest PRIVATE ${DEPENDENCIES} gym)
target_include_directories(GymTest PRIVATE .. ../../thirdparty/tinyndarray)

if(${ADD_PROCGEN})
    # include qt5
    set(CMAKE_PREFIX_PATH ${SAM_PATH}/thirdparty/qt)
    find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)
    target_link_libraries(GymTest PUBLIC Qt5::Core Qt5::Widgets Qt5::Gui)
ENDIF()
