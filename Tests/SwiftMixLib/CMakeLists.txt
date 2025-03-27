cmake_minimum_required(VERSION 3.24)
project(SwiftMixLib C CXX Swift)

# Swift on Windows only provides a release runtime.
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)

add_library(SwiftMixedLib lib.c lib.cpp lib.swift)
add_executable(Swifty main.swift)
target_link_libraries(Swifty PUBLIC SwiftMixedLib)

add_executable(c_main main.c)
target_link_libraries(c_main PUBLIC SwiftMixedLib)

if(WIN32)
  # TODO: On macOS and Linux, Swift has mechanism for determining what libraries
  #       an object, or objects from a static archive, need to link against,
  #       which is how the Swift driver is able to determine that `c_main`
  #       needs to link swiftCore. Windows does not have this mechanism.
  #       Eventually CMake should learn how to do this, explicitly forwarding
  #       the required library to the link command when linking a static
  #       archive containing Swift sources into something else, even if the
  #       linker language is Swift.
  target_link_libraries(c_main PRIVATE swiftCore)
endif()
