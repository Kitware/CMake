
find_package(NoPrefix NO_CMAKE_INSTALL_PREFIX)
if(NoPrefix_FOUND)
  message(FATAL_ERROR "Should not find package when NO_CMAKE_INSTALL_PREFIX specified")
endif()

set(CMAKE_FIND_USE_INSTALL_PREFIX ON)
find_package(NoPrefix)
if(NOT NoPrefix_FOUND)
  message(FATAL_ERROR "Should always find package when CMAKE_FIND_USE_INSTALL_PREFIX is enabled")
endif()

unset(CMAKE_FIND_USE_INSTALL_PREFIX)
unset(NoPrefix_DIR CACHE)

find_package(NoPrefix REQUIRED)
