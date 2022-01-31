set(CMAKE_PREFIX_PATH
  ${CMAKE_SOURCE_DIR}/PackageRoot/foo/cmake_root
  ${CMAKE_SOURCE_DIR}/PackageRoot/foo/env_root
  )
set(CMAKE_IGNORE_PATH
  ${CMAKE_SOURCE_DIR}/PackageRoot//foo/cmake_root// # Test double slashes
  ${CMAKE_SOURCE_DIR}/PackageRoot/foo/env_root/cmake
  )
find_package(Bar QUIET CONFIG)
if(Bar_FOUND)
  message(FATAL_ERROR "Bar should not be found, was found in ${Bar_DIR}")
endif()
