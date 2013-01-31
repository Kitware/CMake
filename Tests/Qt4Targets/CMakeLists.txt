cmake_minimum_required(VERSION 2.8)

project(Qt4Targets)

find_package(Qt4 REQUIRED)

add_executable(Qt4Targets WIN32 main.cpp)
target_link_libraries(Qt4Targets Qt4::QtGui)

if (WIN32)
  target_link_libraries(Qt4Targets Qt4::qtmain)
endif()

set_property(TARGET Qt4Targets APPEND PROPERTY
  INCLUDE_DIRECTORIES
    $<TARGET_PROPERTY:Qt4::QtGui,INTERFACE_INCLUDE_DIRECTORIES>
)
set_property(TARGET Qt4Targets APPEND PROPERTY
  COMPILE_DEFINITIONS
    $<TARGET_PROPERTY:Qt4::QtGui,INTERFACE_COMPILE_DEFINITIONS>
)
