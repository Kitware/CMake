set(min_ver 2.7.20090305)
cmake_minimum_required(VERSION ${min_ver})

if("${CMAKE_VERSION}" VERSION_LESS "${min_ver}")
  message(FATAL_ERROR
    "CMAKE_VERSION=[${CMAKE_VERSION}] is less than [${min_ver}]")
else()
  message("CMAKE_VERSION=[${CMAKE_VERSION}] is not less than [${min_ver}]")
endif()

set(v 1.2.3.4.5.6.7)
if("${v}.8" VERSION_LESS "${v}.9")
  message(STATUS "${v}.8 is less than ${v}.9")
else()
  message(FATAL_ERROR "${v}.8 is not less than ${v}.9?")
endif()
