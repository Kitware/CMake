set(CMAKE_CONFIGURATION_TYPES Debug Release MinSizeRel RelWithDebInfo)
cmake_policy(SET CMP0141 NEW)
enable_language(C)
enable_language(CXX)

add_library(default-C empty.c)
add_library(default-CXX empty.cxx)

set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "")
add_library(empty-C empty.c)
add_library(empty-CXX empty.cxx)

set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "Embedded")
add_library(Embedded-C empty.c)
add_library(Embedded-CXX empty.cxx)

set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "ProgramDatabase")
add_library(ProgramDatabase-C empty.c)
add_library(ProgramDatabase-CXX empty.cxx)

add_library(EditAndContinue-C empty.c)
set_property(TARGET EditAndContinue-C PROPERTY MSVC_DEBUG_INFORMATION_FORMAT "EditAndContinue")
add_library(EditAndContinue-CXX empty.cxx)
set_property(TARGET EditAndContinue-CXX PROPERTY MSVC_DEBUG_INFORMATION_FORMAT "EditAndContinue")
