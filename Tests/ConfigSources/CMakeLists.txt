
cmake_minimum_required(VERSION 3.0)

project(ConfigSources)

add_library(iface INTERFACE)
set_property(TARGET iface PROPERTY INTERFACE_SOURCES
  iface_src.cpp
  $<$<CONFIG:Debug>:iface_debug_src.cpp>
  $<$<CONFIG:Release>:does_not_exist.cpp>
)

add_executable(ConfigSources
  $<$<CONFIG:Debug>:main.cpp>
  $<$<CONFIG:Release>:does_not_exist.cpp>
)
target_link_libraries(ConfigSources iface)
