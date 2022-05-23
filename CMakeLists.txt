cmake_minimum_required(VERSION 3.17)

set(CMAKE_TOOLCHAIN_FILE $ENV{HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")
set(Torch_DIR $ENV{HOME}/libtorch/share/cmake/Torch CACHE STRING "Path to Torch ROOT DIR")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS} -ffat-lto-objects -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

project(gym)

set(THIRD_PARTY ${CMAKE_CURRENT_LIST_DIR}/third_party)
set(Boost_USE_STATIC_LIBS   ON)
set(CMAKE_CXX_STANDARD 20)
set(BUILD_TEST ON)

option(ADD_BOX2D OFF)
option(ADD_ATARI OFF)
option(ADD_PYTHON ON)
option(ADD_PROCGEN OFF)


include_directories(third_party ${OPENGL_INCLUDE_DIR} ${THIRD_PARTY}/viz_doom/include)

find_package(box2d CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 )

set(OpenCV_DIR ${SAM_PATH}/thirdparty/open_cv/lib/cmake/opencv4)
find_package(OpenCV REQUIRED core highgui)

find_package(Torch REQUIRED)
find_package(Boost COMPONENTS filesystem thread system date_time chrono regex   REQUIRED)
find_package(Threads REQUIRED)

set(DEPENDENCIES ${Boost_LIBRARIES}  ${ALE_LIB}
		 OpenGL::GL glfw box2d::box2d
		${OpenCV_LIBS} ${TORCH_LIBRARIES} ${LIBVIZDOOM}
		${CMAKE_THREAD_LIBS_INIT}
		)

add_library(gym STATIC "" common/tensor_adapter.cpp custom/dm_lab/lab.cpp)

target_link_libraries(gym PRIVATE ${DEPENDENCIES})
target_include_directories(gym PRIVATE .  ${OPENGL_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS} ${TORCH_INCLUDE_DIRS})
target_compile_options(gym PRIVATE -fvisibility=hidden)
set_target_properties(gym PROPERTIES CXX_VISIBILITY_PRESET "hidden" CUDA_VISIBILITY_PRESET "hidden")

add_subdirectory(common)
add_subdirectory(spaces)
add_subdirectory(classic_control)
add_subdirectory(wrappers)
add_subdirectory(custom)

if(${ADD_ATARI})
	add_subdirectory(atari)
endif()

if(${ADD_BOX2D})
	add_subdirectory(box2d)
endif()

if(${BUILD_TEST})
	find_package(Catch2 CONFIG REQUIRED)
	add_subdirectory(test)
	target_link_libraries(GymTest PRIVATE ${DEPENDENCIES} gym)
	target_include_directories(GymTest PRIVATE .)
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
