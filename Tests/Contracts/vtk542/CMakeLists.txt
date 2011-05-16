# The VTK external project for CMake
# ---------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8)
project(vtk542)
include(ExternalProject)


set(vtk_source "${CMAKE_CURRENT_BINARY_DIR}/VTK-source")
set(vtk_binary "${CMAKE_CURRENT_BINARY_DIR}/VTK-build")

ExternalProject_Add(VTK
  DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}
  URL "http://www.vtk.org/files/release/5.4/vtk-5.4.2.tar.gz"
  URL_MD5 c2c797091d4b2128d9a1bd32c4b78227
  SOURCE_DIR ${vtk_source}
  BINARY_DIR ${vtk_binary}
  CMAKE_GENERATOR  "${CMAKE_GENERATOR}"
  CMAKE_ARGS
  -DBUILD_EXAMPLES:BOOL=ON
  -DBUILD_TESTING:BOOL=ON
  INSTALL_COMMAND ""
  )
# make it so that each build will run make in the VTK build tree
ExternalProject_Add_Step(VTK forcebuild
  COMMAND ${CMAKE_COMMAND}
  -E remove ${CMAKE_CURRENT_BUILD_DIR}/VTK-prefix/src/VTK-stamp/VTK-build
  DEPENDEES configure
  DEPENDERS build
  ALWAYS 1
  )
