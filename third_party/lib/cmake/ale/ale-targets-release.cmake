#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ale::ale-lib" for configuration "Release"
set_property(TARGET ale::ale-lib APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(ale::ale-lib PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libale.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS ale::ale-lib )
list(APPEND _IMPORT_CHECK_FILES_FOR_ale::ale-lib "${_IMPORT_PREFIX}/lib/libale.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
