set(CMAKE_PREFIX_PATH
  ${CMAKE_SOURCE_DIR}/PackageRoot/foo/cmake_root
  ${CMAKE_SOURCE_DIR}/PackageRoot/foo/env_root
  )
set(CMAKE_IGNORE_PREFIX_PATH
  ${CMAKE_SOURCE_DIR}/PackageRoot//foo/cmake_root// # Test double slashes
  )
set(CMAKE_SYSTEM_IGNORE_PREFIX_PATH
  ${CMAKE_SOURCE_DIR}/PackageRoot/foo/env_root
  )
find_package(Bar QUIET CONFIG)
if(Bar_FOUND)
  message(SEND_ERROR "Bar should not be found, was found in ${Bar_DIR}")
endif()

set(CMAKE_PREFIX_PATH)
set(CMAKE_FIND_ROOT_PATH
  ${CMAKE_SOURCE_DIR}/PackageRoot/foo/cmake_root
  ${CMAKE_SOURCE_DIR}/PackageRoot/foo/env_root
  )
set(CMAKE_IGNORE_PREFIX_PATH /)
set(CMAKE_SYSTEM_IGNORE_PREFIX_PATH)
find_package(Bar2 NAMES Bar QUIET CONFIG)
if(Bar2_FOUND)
  message(SEND_ERROR "Bar2 should not be found, was found in ${Bar2_DIR}")
endif()
