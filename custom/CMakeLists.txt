
if(MSVC)
    message("All Custom Environments are deactivated in Windows")
else()
    add_subdirectory(box_world)
    add_subdirectory(vizdoom)
    add_subdirectory(dm_lab)
endif()