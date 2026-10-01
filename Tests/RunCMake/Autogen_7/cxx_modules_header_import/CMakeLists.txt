cmake_minimum_required(VERSION 3.28)
project(cxx_modules_header_import CXX)

find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core)

add_executable(app main.cpp obj.h)
target_sources(app
  PRIVATE FILE_SET CXX_MODULES FILES
    mod.cppm)

set_target_properties(app PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  AUTOMOC ON)

target_link_libraries(app PRIVATE Qt${with_qt_version}::Core)
