cmake_minimum_required(VERSION 3.17)

set(CMAKE_TOOLCHAIN_FILE $ENV{HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")

project(gym)

set(THIRD_PARTY ${CMAKE_CURRENT_LIST_DIR}/third_party)
set(Torch_DIR $ENV{HOME}/libtorch/share/cmake/Torch)
set(Boost_USE_STATIC_LIBS   ON)
set(CMAKE_CXX_STANDARD 20)
set(BUILD_TEST ON)

option(ADD_BOX2D OFF)
option(ADD_ATARI OFF)

include_directories(third_party ${OPENGL_INCLUDE_DIR} ${THIRD_PARTY}/linux/ViZDoom/include)

find_package(Catch2 CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 )
find_package(OpenCV REQUIRED )
find_package(Torch REQUIRED)
find_package(Boost COMPONENTS filesystem thread system date_time chrono regex fiber context REQUIRED)
find_package(Threads REQUIRED)
find_package(pybind11 CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)

set(DEPENDENCIES ${Boost_LIBRARIES}
		Catch2::Catch2 OpenGL::GL glfw box2d::box2d
		${OpenCV_LIBS} ${TORCH_LIBRARIES} ${LIBVIZDOOM}  ${CMAKE_THREAD_LIBS_INIT}
		pybind11::embed
		)

set(GYM_TEST_FILES test/classic_control.cpp
		 test/atari_env.cpp test/wrappers.cpp
#		test/box_world.cpp  test/rendering.cpp test/spaces.cpp
		)

add_library(gym STATIC "" common/tensor_adapter.cpp custom/dm_lab/lab.cpp)

target_link_libraries(gym PRIVATE ${DEPENDENCIES})
target_include_directories(gym PRIVATE . ale)
target_compile_options(gym PRIVATE -fvisibility=hidden)
set_target_properties(gym PROPERTIES CXX_VISIBILITY_PRESET "hidden" CUDA_VISIBILITY_PRESET "hidden")

add_subdirectory(common)
add_subdirectory(spaces)
add_subdirectory(classic_control)
add_subdirectory(wrappers)

if(${ADD_BOX2D})
	add_subdirectory(box2d)
endif()

if(${ADD_ATARI})
	add_subdirectory(atari)
endif()

#add_subdirectory(custom)
add_subdirectory(python_gym)

install (TARGETS ${PROJECT_NAME}
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib)

install (FILES env.h DESTINATION include/gym )

add_executable(gym_main main.cpp)
target_link_libraries(gym_main PRIVATE ${DEPENDENCIES} gym)
target_include_directories(gym_main PRIVATE .)

if(${BUILD_TEST})
	add_executable(GymTest ${GYM_TEST_FILES})
	target_link_libraries(GymTest PRIVATE ${DEPENDENCIES} gym)
	target_include_directories(GymTest PRIVATE .)
endif()


