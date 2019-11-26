cmake_minimum_required(VERSION 3.0)
project(ConfigSources CXX)

add_library(iface INTERFACE)
target_sources(iface INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/iface_src.cpp"
  "$<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/iface_debug_src.cpp>"
  "$<$<NOT:$<CONFIG:Debug>>:${CMAKE_CURRENT_SOURCE_DIR}/iface_other_src.cpp>"
  "$<$<CONFIG:NotAConfig>:${CMAKE_CURRENT_SOURCE_DIR}/does_not_exist.cpp>"
  )
target_compile_definitions(iface INTERFACE
  "$<$<CONFIG:Debug>:CFG_DEBUG>"
  "$<$<NOT:$<CONFIG:Debug>>:CFG_OTHER>"
  )

add_executable(ConfigSources
  $<$<CONFIG:Debug>:main_debug.cpp>
  $<$<NOT:$<CONFIG:Debug>>:main_other.cpp>
  $<$<CONFIG:NotAConfig>:does_not_exist.cpp>
  )
target_link_libraries(ConfigSources iface)
