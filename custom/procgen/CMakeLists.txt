#
#if(APPLE)
#  # clang defaults to 20 errors, but usually only the first one is useful
#  add_compile_options(-ferror-limit=1)
#  # only produce errors on mac where development is done
#  add_compile_options(-Werror -Wextra -Wshadow -Wall -Wformat=2 -Wundef -Wvla -Wmissing-include-dirs -Wnon-virtual-dtor -Wno-unused-parameter)
#endif()
#
#if(MSVC)
#  # allow strcpy
#  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
#endif()
#
#if (APPLE OR UNIX)
#  if(PROCGEN_PACKAGE)
#    # compile for the minimum spec processor
#    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -march=ivybridge")
#  else()
#    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -march=native")
#  endif()
#
#  # leave frame pointers so that profiling tools will still work
#  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer")
#endif()


add_subdirectory(src)


set(CMAKE_PREFIX_PATH ${SAM_PATH}/thirdparty/qt)
find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)
target_link_libraries(procgen PRIVATE Qt5::Core Qt5::Widgets Qt5::Gui ${OpenCV_LIBS})

target_include_directories(procgen PRIVATE src)
target_sources(${PROJECT_NAME} PRIVATE procgen.cpp)
target_link_libraries( ${PROJECT_NAME} PUBLIC procgen  )
target_include_directories(${PROJECT_NAME} PUBLIC ..)

macro(configure_files srcDir destDir)
    message(STATUS "Configuring directory ${destDir}")
    make_directory(${destDir})

    file(GLOB templateFiles RELATIVE ${srcDir} ${srcDir}/*)
    foreach(templateFile ${templateFiles})
        set(srcTemplatePath ${srcDir}/${templateFile})
        if(NOT IS_DIRECTORY ${srcTemplatePath})
            message(STATUS "Configuring file ${templateFile}")
            configure_file(
                    ${srcTemplatePath}
                    ${destDir}/${templateFile}
                    @ONLY)
        endif(NOT IS_DIRECTORY ${srcTemplatePath})
    endforeach(templateFile)
endmacro(configure_files)


configure_files(${CMAKE_CURRENT_LIST_DIR}/data,  ${CMAKE_BINARY_DIR}/exec)
configure_files(${CMAKE_CURRENT_LIST_DIR}/data,  ${CMAKE_BINARY_DIR})