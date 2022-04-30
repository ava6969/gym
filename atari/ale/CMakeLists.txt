
# Include src/ and cmake binary directory (for version.hpp)
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# Project specific target properties
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

# Compiler options
add_compile_options(
  "$<$<CONFIG:RELEASE>:-O3>"
  "$<$<CONFIG:DEBUG>:-O0>"
  "$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall>"
  "$<$<CXX_COMPILER_ID:MSVC>:/W4>")

# Dependencies
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    ZLIB::ZLIB
    Threads::Threads)

if(SDL_SUPPORT)
  option(SDL_DYNLOAD "Dynamically load SDL" OFF)
  find_package(SDL2 CONFIG REQUIRED)

  # VCPKG doesn't include SDL2_INCLUDE_DIRS
  if(VCPKG_TOOLCHAIN)
    find_path(SDL2_INCLUDE_DIRS SDL.h PATH_SUFFIXES SDL2)
  endif()

  target_compile_definitions(ale
    PUBLIC
      ALE_SDL_SUPPORT
    PRIVATE
      SDL_SUPPORT
      $<$<BOOL:${SDL_DYNLOAD}>:SDL_DYNLOAD>)
  target_link_libraries(ale PRIVATE $<$<NOT:$<BOOL:${SDL_DYNLOAD}>>:SDL2::SDL2>)
  target_include_directories(ale PRIVATE ${SDL2_INCLUDE_DIRS})
endif()

# Add submodules
add_subdirectory(common)
add_subdirectory(emucore)
add_subdirectory(environment)
add_subdirectory(games)

# C++ Library
target_sources(${PROJECT_NAME} PRIVATE  ale_interface.cpp)


