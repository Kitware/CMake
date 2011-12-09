cmake_minimum_required(VERSION 2.8)
project(VSGNUFortran)
# force the executable to be put out of Debug/Release dir
# because gmake build of fortran will not be in a config
# directory, and for easier testing we want the exe and .dll
# to be in the same directory.
if(CMAKE_CONFIGURATION_TYPES)
  foreach(config ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER "${config}" config)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${config}
      "${PROJECT_BINARY_DIR}/bin")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config}
      "${PROJECT_BINARY_DIR}/bin")
  endforeach()
else()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()

include(CMakeAddFortranSubdirectory)
# add the fortran subdirectory as a fortran project
# the subdir is fortran, the project is FortranHello
cmake_add_fortran_subdirectory(fortran
  PROJECT FortranHello  # project name in toplevel CMakeLists.txt
  ARCHIVE_DIR ../bin # .lib location relative to root binary tree
  RUNTIME_DIR ../bin # .dll location relative to root binary tree
  LIBRARIES hello world # target libraries created
  CMAKE_COMMAND_LINE -DEXECUTABLE_OUTPUT_PATH=../bin
                     -DLIBRARY_OUTPUT_PATH=../bin
  LINK_LIBRARIES  # link interface libraries
   LINK_LIBS hello world  # hello needs world to link
   )

include_directories(${VSGNUFortran_BINARY_DIR}/fortran)
add_subdirectory(c_code)
# use a cmake script to run the executable so that PATH
# can be set with the MinGW/bin in it, and the fortran
# runtime libraries can be found.
configure_file(runtest.cmake.in runtest.cmake @ONLY)
