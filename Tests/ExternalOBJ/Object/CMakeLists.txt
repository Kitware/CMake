cmake_minimum_required (VERSION 2.6)
project(Object)
if(APPLE)
  # set _CMAKE_OSX_MACHINE to umame -m
  execute_process(COMMAND uname -m OUTPUT_VARIABLE _CMAKE_OSX_MACHINE OUTPUT_STRIP_TRAILING_WHITESPACE)
  # check for Power PC and change to ppc
  if("${_CMAKE_OSX_MACHINE}" MATCHES "Power")
    set(_CMAKE_OSX_MACHINE ppc)
  endif()
  set(CMAKE_OSX_ARCHITECTURES ${_CMAKE_OSX_MACHINE})
endif()

add_executable(external external_object.cxx external_main.cxx)
