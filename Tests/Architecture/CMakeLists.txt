cmake_minimum_required(VERSION 2.8)
project(Architecture C)

add_library(foo foo.c)
if(CMAKE_OSX_ARCHITECTURES)
  get_property(archs TARGET foo PROPERTY OSX_ARCHITECTURES)
  if(NOT "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "${archs}")
    message(FATAL_ERROR
      "OSX_ARCHITECTURES property not initialized by CMAKE_OSX_ARCHITECTURES.\n"
      "Expected [${CMAKE_OSX_ARCHITECTURES}], got [${archs}]."
      )
  endif()
endif()
set_property(TARGET foo PROPERTY OSX_ARCHITECTURES i386)
set_property(TARGET foo PROPERTY OSX_ARCHITECTURES_DEBUG ppc)

add_executable(bar bar.c)
target_link_libraries(bar foo)
set_property(TARGET bar PROPERTY OUTPUT_NAME Architecture)
set_property(TARGET bar PROPERTY OSX_ARCHITECTURES ppc)
set_property(TARGET bar PROPERTY OSX_ARCHITECTURES_DEBUG i386)
