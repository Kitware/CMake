
find_package(Bar QUIET CONFIG NO_CMAKE_INSTALL_PREFIX)
if(Bar_FOUND)
  message(SEND_ERROR "Bar should not be found, was found in ${Bar_DIR}")
endif()

set(CMAKE_FIND_USE_INSTALL_PREFIX OFF)
find_package(Bar QUIET CONFIG)
if(Bar_FOUND)
  message(SEND_ERROR "Bar should not be found, was found in ${Bar_DIR}")
endif()

set(CMAKE_FIND_USE_INSTALL_PREFIX ON)
find_package(Bar QUIET CONFIG)
if(NOT Bar_FOUND)
  message(SEND_ERROR "Bar should be found via CMAKE_INSTALL_PREFIX")
endif()
