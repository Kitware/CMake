cmake_minimum_required(VERSION 2.8)
project(Architecture C)

set(archs i386 ppc)

if(NOT "${DARWIN_MAJOR_VERSION}.${DARWIN_MINOR_VERSION}" VERSION_LESS 10.0)
  # Snow Leopard or later, use modern architectures as defaults
  # Arch 'ppc' no longer works: tools no longer available starting with Xcode 4
  set(archs i386 x86_64)
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
endif()

list(LENGTH archs archs_len)
if(archs_len GREATER 1)
  list(GET archs 0 arch0)
  list(GET archs 1 arch1)
else()
  set(arch0 ${archs})
  set(arch1 ${archs})
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
