# Prepare environment and variables
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH TRUE)
if(WIN32)
    set(ENV{CMAKE_PREFIX_PATH} "${CMAKE_CURRENT_SOURCE_DIR}\\pc-bletch")
else()
    set(ENV{CMAKE_PREFIX_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/pc-bletch")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(BLETCH QUIET bletch)

if (NOT BLETCH_FOUND)
  message(FATAL_ERROR "Failed to find embedded package bletch via CMAKE_PREFIX_PATH")
endif ()

pkg_get_variable(bletchvar bletch exec_prefix DEFINE_VARIABLES prefix=customprefix)
if (NOT bletchvar STREQUAL "customprefix")
  message(FATAL_ERROR "Failed to fetch variable exec_prefix from embedded package bletch with prefix overridden to customprefix")
endif ()
