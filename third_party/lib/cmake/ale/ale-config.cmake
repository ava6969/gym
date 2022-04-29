
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was ale-config.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "../../../ale" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

set(SDL_SUPPORT OFF)
set(SDL_DYNLOAD )

include(CMakeFindDependencyMacro)
find_dependency(ZLIB REQUIRED)
find_dependency(Threads REQUIRED)

if(SDL_SUPPORT AND NOT SDL_DYNLOAD)
  find_dependency(SDL2 REQUIRED)
endif()

include("ale-targets.cmake")
