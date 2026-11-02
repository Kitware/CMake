cmake_minimum_required(VERSION 3.23)
project(embt_subsystem CXX)

if(NOT DEFINED CASE OR CASE STREQUAL "plain")
  add_executable(console console.cpp)   # console (default subsystem)
  add_executable(gui WIN32 gui.cpp)     # GUI subsystem
elseif(CASE STREQUAL "genex")
  # WIN32_EXECUTABLE as a per-config genex: GUI in Release, console otherwise.
  # Built in Release, so it is GUI and must NOT see __CONSOLE__ (gui.cpp asserts
  # that).  A non-genex read of the property would pick console here and define
  # __CONSOLE__, tripping the #error -- so this locks compile==link per config.
  add_executable(genexgui gui.cpp)
  set_target_properties(genexgui PROPERTIES WIN32_EXECUTABLE "$<CONFIG:Release>")
endif()
