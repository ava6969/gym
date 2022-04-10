cmake_minimum_required(VERSION 3.17)

set(CMAKE_TOOLCHAIN_FILE $ENV{HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")
set(Torch_DIR $ENV{HOME}/libtorch/share/cmake/Torch CACHE STRING "Path to Torch ROOT DIR")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")

project(gym)

set(THIRD_PARTY ${CMAKE_CURRENT_LIST_DIR}/third_party)
set(Boost_USE_STATIC_LIBS   ON)
set(CMAKE_CXX_STANDARD 20)
set(BUILD_TEST ON)

option(ADD_BOX2D OFF)
option(ADD_ATARI OFF)

include_directories(third_party ${OPENGL_INCLUDE_DIR} ${THIRD_PARTY}/viz_doom/include)

find_package(pybind11 CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 )

set(OpenCV_DIR ${SAM_PATH}/thirdparty/open_cv/lib/cmake/opencv4)
find_package(OpenCV REQUIRED core highgui)

find_package(Torch REQUIRED)
find_package(Boost COMPONENTS filesystem thread system date_time chrono regex   REQUIRED)
find_package(Threads REQUIRED)

set(DEPENDENCIES ${Boost_LIBRARIES}  ${ALE_LIB}
		Catch2::Catch2 OpenGL::GL glfw box2d::box2d
		${OpenCV_LIBS} ${TORCH_LIBRARIES} ${LIBVIZDOOM}  ${CMAKE_THREAD_LIBS_INIT}
		pybind11::embed )

set(GYM_TEST_FILES test/classic_control.cpp test/atari_env.cpp test/wrappers.cpp)

message(STATUS "TORCH_LIBRARIES: ${TORCH_LIBRARIES}")
message(STATUS "TORCH_INCLUDE_DIRS: ${TORCH_INCLUDE_DIRS}")
message(STATUS "GL_INCLUDES: ${OPENGL_INCLUDE_DIR}")

add_library(gym STATIC "" common/tensor_adapter.cpp custom/dm_lab/lab.cpp)

target_link_libraries(gym PRIVATE ${DEPENDENCIES})
target_include_directories(gym PRIVATE .  ${OPENGL_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS} ${TORCH_INCLUDE_DIRS})
target_compile_options(gym PRIVATE -fvisibility=hidden)
set_target_properties(gym PROPERTIES CXX_VISIBILITY_PRESET "hidden" CUDA_VISIBILITY_PRESET "hidden")

add_subdirectory(common)
add_subdirectory(spaces)
add_subdirectory(classic_control)
add_subdirectory(wrappers)

if(${ADD_ATARI})
	add_subdirectory(atari)
endif()

if(${ADD_BOX2D})
	add_subdirectory(box2d)
endif()

add_subdirectory(python_gym)

install (TARGETS ${PROJECT_NAME}
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib)
install (FILES env.h DESTINATION include/gym )

add_executable(gym_main main.cpp)
target_link_libraries(gym_main PRIVATE ${DEPENDENCIES} gym)
target_include_directories(gym_main PRIVATE . )

if(${BUILD_TEST})
	add_executable(GymTest ${GYM_TEST_FILES})
	target_link_libraries(GymTest PRIVATE ${DEPENDENCIES} gym)
	target_include_directories(GymTest PRIVATE .)
endif()