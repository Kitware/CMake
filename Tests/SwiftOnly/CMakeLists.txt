cmake_minimum_required(VERSION 3.3)
if(POLICY CMP0126)
  cmake_policy(SET CMP0126 NEW)
endif()

# NOTE: Force the Release mode configuration as there are some issues with the
# debug information handling on macOS on certain Xcode builds.
if(NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE)
endif()

# NOTE: enable shared libraries by default.  Older Xcode releases do not play
# well with static libraries, and Windows does not currently support static
# libraries in Swift.
set(BUILD_SHARED_LIBS YES)

project(SwiftOnly Swift)

if(NOT XCODE_VERSION VERSION_LESS 10.2)
  set(CMAKE_Swift_LANGUAGE_VERSION 5.0)
elseif(NOT XCODE_VERSION VERSION_LESS 8.0)
  set(CMAKE_Swift_LANGUAGE_VERSION 3.0)
endif()

set(CMAKE_Swift_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/swift)

add_executable(SwiftOnly main.swift)

add_library(L L.swift)

add_library(M M.swift)
target_link_libraries(M PUBLIC
  L)

add_library(N N.swift)
target_link_libraries(N PUBLIC
  M)

# Dummy to make sure generation works with such targets.
add_library(SwiftIface INTERFACE)
target_link_libraries(SwiftOnly PRIVATE SwiftIface)
