cmake_minimum_required(VERSION 3.28)
project(cxx_modules_no_macro_automoc CXX)

find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core)

# The module unit pulls in no Qt at all, so this builds with any compiler that
# can scan C++ modules, and with any Qt version: nothing is generated for a
# module unit without a meta-object macro, while object.h is still moc'd.
add_executable(app main.cpp object.h)
target_sources(app
  PRIVATE FILE_SET CXX_MODULES FILES
    mod.cppm)

set_target_properties(app PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  AUTOMOC ON)

target_link_libraries(app PRIVATE Qt${with_qt_version}::Core)
