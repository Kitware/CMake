# This CMakeLists.txt is part of the subproject B (ExternalProject_Add).

cmake_minimum_required(VERSION 3.20)
project(subpro_d_executable LANGUAGES CXX)

find_package(static_lib_project REQUIRED)
find_package(shared_lib_project REQUIRED)
find_package(c2_lib_project REQUIRED)

add_executable(the_executable
  "src/main.cpp"
)

target_link_libraries(the_executable PRIVATE
  the_static_lib
  the_shared_lib
  the_c2_lib
)

install(
  TARGETS
    the_executable
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
