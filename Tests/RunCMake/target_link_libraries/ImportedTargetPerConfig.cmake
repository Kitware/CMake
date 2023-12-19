enable_language(C)
get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _isMultiConfig AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

add_library(StaticImported STATIC IMPORTED)

# Test with no IMPORTED_CONFIGURATIONS, which works if the
# imported target provides all exact-name configurations
# built by this project.  See issue #25515.
set_target_properties(StaticImported PROPERTIES
  IMPORTED_LOCATION_DEBUG "a"
  IMPORTED_LOCATION_RELEASE "b"
  IMPORTED_LOCATION_MINSIZEREL "c"
  IMPORTED_LOCATION_RELWITHDEBINFO "d"
  )

add_library(StaticLib STATIC empty.c)

# The Xcode generator queries imported targets for system
# include directories, but without any specific config.
set_source_files_properties(empty.c PROPERTIES
  INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}"
  )

target_link_libraries(StaticLib PRIVATE StaticImported)
