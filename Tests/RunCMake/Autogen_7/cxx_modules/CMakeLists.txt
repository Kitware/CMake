cmake_minimum_required(VERSION 3.28)
project(cxx_modules_automoc CXX)

find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core)

# QObject pulled into a module unit triggers exposure violations under GCC
# (QTBUG-142513); skip cleanly there, matching qtbase's own module tests.
# With older Qt no module unit is ever compiled here, because the build is
# expected to fail in AUTOMOC first, so the exclusion does not apply.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
   Qt${with_qt_version}Core_VERSION VERSION_GREATER_EQUAL "6.13")
    return()
endif()

add_executable(app main.cpp mod-impl.cpp)
target_sources(app
  PRIVATE FILE_SET CXX_MODULES FILES
    mod.cppm
    mod-part.cppm
    mod-internal.cppm
    mod-empty.cppm)

set_target_properties(app PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  AUTOMOC ON)

target_link_libraries(app PRIVATE Qt${with_qt_version}::Core)
