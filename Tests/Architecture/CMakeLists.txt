cmake_minimum_required(VERSION 3.5)
project(Architecture C)

if (CMake_TEST_XCODE_VERSION VERSION_GREATER_EQUAL 12)
  # Since Xcode 12 we have two architectures again: arm64 and x86_64.
  set(arch0 x86_64)
  set(arch1 arm64)
elseif(CMake_TEST_XCODE_VERSION VERSION_GREATER_EQUAL 10)
  # Xcode 10 / 11 do not have two supported architectures for the host.
  message(STATUS "Skip test x86_64 only")
  return()
elseif(CMake_TEST_XCODE_VERSION VERSION_GREATER_EQUAL 4)
  # Xcode 4 no longer supports ppc, but does support x86_64.
  set(arch0 i386)
  set(arch1 x86_64)
else()
  set(arch0 i386)
  set(arch1 ppc)
endif()

add_library(foo foo.c)
if(CMAKE_OSX_ARCHITECTURES)
  get_property(archs TARGET foo PROPERTY OSX_ARCHITECTURES)
  if(NOT "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "${archs}")
    message(FATAL_ERROR
      "OSX_ARCHITECTURES property not initialized by CMAKE_OSX_ARCHITECTURES.\n"
      "Expected [${CMAKE_OSX_ARCHITECTURES}], got [${archs}]."
      )
  endif()
  list(LENGTH archs archs_len)
  if(archs_len GREATER 1)
    list(GET archs 0 arch0)
    list(GET archs 1 arch1)
  endif()
endif()

message("archs='${archs}'")
message("arch0='${arch0}'")
message("arch1='${arch1}'")

set_property(TARGET foo PROPERTY OSX_ARCHITECTURES ${arch0})
set_property(TARGET foo PROPERTY OSX_ARCHITECTURES_DEBUG ${arch1})

add_executable(bar bar.c)
target_link_libraries(bar foo)

set_property(TARGET bar PROPERTY OUTPUT_NAME Architecture)
set_property(TARGET bar PROPERTY OSX_ARCHITECTURES ${arch1})
set_property(TARGET bar PROPERTY OSX_ARCHITECTURES_DEBUG ${arch0})
