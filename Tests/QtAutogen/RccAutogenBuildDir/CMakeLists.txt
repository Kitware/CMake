cmake_minimum_required(VERSION 3.16)
project(RccAutogenBuildDir)
include("../AutogenCoreTest.cmake")

set(PROJECTS_ROOT ${CMAKE_BINARY_DIR})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

macro(set_build_type_dependent varName debugValue releaseValue
      relWithDebInfoValue minSizeRelValue)

  if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(${varName} ${debugValue})
  elseif(CMAKE_BUILD_TYPE MATCHES Release)
    set(${varName} ${releaseValue})
  elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    set(${varName} ${relWithDebInfoValue})
  elseif(CMAKE_BUILD_TYPE MATCHES MinSizeRel)
    set(${varName} ${minSizeRelValue})
  endif()
endmacro()

set_build_type_dependent(AUTOGEN_DIR agd agr ags agm)
add_library(testlib SHARED lib.h lib.cpp resource.qrc)
set_target_properties(testlib PROPERTIES AUTOGEN_BUILD_DIR "${PROJECTS_ROOT}/${AUTOGEN_DIR}/testlib_ag")
target_link_libraries(testlib ${QT_LIBRARIES})

set_build_type_dependent(AUTOGEN_DIR agd agr ags agm)
add_executable(autorcctest main.cpp lib.h)
set_target_properties(autorcctest PROPERTIES AUTOGEN_BUILD_DIR "${PROJECTS_ROOT}/${AUTOGEN_DIR}/autorcctest_ag")
target_link_libraries(autorcctest ${QT_LIBRARIES} testlib)
