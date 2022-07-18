project(s2clientprotocol)
cmake_minimum_required(VERSION 2.8.12)
set (CMAKE_CXX_STANDARD 11)

find_package(Protobuf REQUIRED)
include_directories(SYSTEM "${PROJECT_BINARY_DIR}")

file(GLOB proto_files "proto/*.proto")

if ("${proto_files}" STREQUAL "")
    message(FATAL_ERROR "Missing root protocol submodule.\nRemove your protocol directory and run:\ngit submodule update --recursive --init")
endif ()

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${proto_files} IMPORT_DIRS .)

add_library(s2clientprotocol STATIC ${PROTO_SRCS} ${proto_files})

target_include_directories(s2clientprotocol PUBLIC
        ${Protobuf_INCLUDE_DIRS}
        ${PROJECT_BINARY_DIR}/..)

target_link_libraries(s2clientprotocol PUBLIC ${Protobuf_LIBRARIES})