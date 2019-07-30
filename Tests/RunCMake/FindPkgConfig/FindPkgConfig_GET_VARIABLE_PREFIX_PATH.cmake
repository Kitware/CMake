# Prepare environment and variables
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH TRUE)
if(WIN32)
    set(ENV{CMAKE_PREFIX_PATH} "${CMAKE_CURRENT_SOURCE_DIR}\\pc-bletch;X:\\this\\directory\\should\\not\\exist\\in\\the\\filesystem")
    set(ENV{PKG_CONFIG_PATH} "C:\\baz")
else()
    set(ENV{CMAKE_PREFIX_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/pc-bletch:/this/directory/should/not/exist/in/the/filesystem")
    set(ENV{PKG_CONFIG_PATH} "/baz")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(BLETCH QUIET bletch)

if (NOT BLETCH_FOUND)
  message(FATAL_ERROR "Failed to find embedded package bletch via CMAKE_PREFIX_PATH")
endif ()

pkg_get_variable(bletchvar bletch jackpot)
if (NOT bletchvar STREQUAL "bletch-lives")
  message(FATAL_ERROR "Failed to fetch variable jackpot from embedded package bletch via CMAKE_PREFIX_PATH")
endif ()
