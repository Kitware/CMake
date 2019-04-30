set(CMAKE_CONFIGURATION_TYPES Debug)
cmake_policy(SET CMP0091 NEW)
enable_language(C)
enable_language(CXX)

add_library(default-C empty.c)
add_library(default-CXX empty.cxx)

set(CMAKE_MSVC_RUNTIME_LIBRARY "")
add_library(empty-C empty.c)
add_library(empty-CXX empty.cxx)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
add_library(MTd-C empty.c)
add_library(MTd-CXX empty.cxx)

add_library(MT-C empty.c)
set_property(TARGET MT-C PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
add_library(MT-CXX empty.cxx)
set_property(TARGET MT-CXX PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
