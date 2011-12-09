cmake_minimum_required(VERSION 2.8)
project(VSGNUFortran)

set(EXECUTABLE_OUTPUT_PATH ${VSGNUFortran_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${VSGNUFortran_BINARY_DIR}/bin)

include(CMakeAddFortranSubdirectory)
# add the lapack subdirectory as a fortran project
# the subdir is lapack, the project is LAPACK
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
