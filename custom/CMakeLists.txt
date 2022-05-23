
if(MSVC)
    message("All Custom Environments are deactivated in Windows")
else()
    add_subdirectory(box_world)

    if(${ADD_PROCGEN})
        # include qt5
        set(CMAKE_PREFIX_PATH ${SAM_PATH}/thirdparty/qt)
        find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)
        target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Widgets Qt5::Gui)

        add_subdirectory(procgen)
    endif()
#    add_subdirectory(vizdoom)
#    add_subdirectory(dm_lab)
endif()