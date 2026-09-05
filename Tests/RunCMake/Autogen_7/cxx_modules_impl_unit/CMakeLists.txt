cmake_minimum_required(VERSION 3.28)
project(cxx_modules_impl_unit_automoc CXX)

find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core)

# moc rejects a meta-object macro in a module implementation unit.  The build
# is expected to fail in moc, so no compiler ever sees the module unit and no
# compiler-specific exclusion is needed here.
add_executable(app main.cpp mod-impl.cpp)
target_sources(app
  PRIVATE FILE_SET CXX_MODULES FILES
    mod.cppm)

set_target_properties(app PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  AUTOMOC ON)

target_link_libraries(app PRIVATE Qt${with_qt_version}::Core)
