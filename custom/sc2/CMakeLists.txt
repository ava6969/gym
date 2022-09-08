add_subdirectory(env)
add_subdirectory(lib)
add_subdirectory(maps)
add_subdirectory(run_configs)
add_subdirectory(test)
target_sources(gym PRIVATE run_parallel.cpp)

find_package(Protobuf REQUIRED)

# Gather the protos.
file(GLOB proto_files
        "../protocol/s2clientprotocol/*.proto"
        )

if ("${proto_files}" STREQUAL "")
    message(FATAL_ERROR "Missing root protocol submodule.\nRemove your protocol directory and run:\ngit submodule update --recursive --init")
endif ()

# Create the output directory for generated protos.
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/generated/s2clientprotocol)

foreach(proto ${proto_files})
    get_filename_component(proto_name ${proto} NAME_WE)
    list(APPEND proto_src ${PROJECT_BINARY_DIR}/generated/s2clientprotocol/${proto_name}.pb.h ${PROJECT_BINARY_DIR}/generated/s2clientprotocol/${proto_name}.pb.cc)
endforeach()

# Now include that directory

add_library(sc2protocol ${proto_src} ${proto_files})
target_link_libraries(sc2protocol ${Protobuf_LIBRARIES})

foreach (proto ${proto_files})
    get_filename_component(proto_name ${proto} NAME)
    get_filename_component(proto_name_we ${proto} NAME_WE)
    set(outputCC ${PROJECT_BINARY_DIR}/generated/s2clientprotocol/${proto_name_we}.pb.cc)
    set(outputH ${PROJECT_BINARY_DIR}/generated/s2clientprotocol/${proto_name_we}.pb.h)

    add_custom_command(
#            DEPENDS Protobuf
            COMMAND /usr/bin/protoc -I=${CMAKE_CURRENT_SOURCE_DIR}/../protocol/ --cpp_out=${PROJECT_BINARY_DIR}/generated ${proto}
            OUTPUT ${outputCC} ${outputH}
    )

endforeach()

target_include_directories(sc2protocol PUBLIC "${PROJECT_BINARY_DIR}/generated")
