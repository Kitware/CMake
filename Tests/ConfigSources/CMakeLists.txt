cmake_minimum_required(VERSION 3.0)
get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _isMultiConfig AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build")
endif()
project(ConfigSources CXX)

# Source file(s) named with the configuration(s).
file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/config_$<CONFIG>.cpp"
  CONTENT "void config_$<CONFIG>() {}\n"
  )

# Per-config sources via INTERFACE_SOURCES.
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
  ${CMAKE_CURRENT_BINARY_DIR}/config_$<CONFIG>.cpp
  )
target_link_libraries(ConfigSources iface)

# Per-config sources via LINK_LIBRARIES.
add_library(iface_debug INTERFACE)
target_sources(iface_debug INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/iface_src.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/iface_debug_src.cpp"
  )
add_library(iface_other INTERFACE)
target_sources(iface_other INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/iface_src.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/iface_other_src.cpp"
  )
add_executable(ConfigSourcesLink main.cpp)
target_compile_definitions(ConfigSourcesLink PRIVATE
  "$<$<CONFIG:Debug>:CFG_DEBUG>"
  "$<$<NOT:$<CONFIG:Debug>>:CFG_OTHER>"
  )
target_link_libraries(ConfigSourcesLink PRIVATE
  "$<$<CONFIG:Debug>:iface_debug>"
  "$<$<NOT:$<CONFIG:Debug>>:iface_other>"
  "$<$<CONFIG:NotAConfig>:iface_does_not_exist>"
  )

# Per-config sources via INTERFACE_LINK_LIBRARIES.
add_library(ConfigSourcesIface INTERFACE)
target_link_libraries(ConfigSourcesIface INTERFACE
  "$<$<CONFIG:Debug>:iface_debug>"
  "$<$<NOT:$<CONFIG:Debug>>:iface_other>"
  "$<$<CONFIG:NotAConfig>:iface_does_not_exist>"
  )
add_executable(ConfigSourcesLinkIface main.cpp)
target_compile_definitions(ConfigSourcesLinkIface PRIVATE
  "$<$<CONFIG:Debug>:CFG_DEBUG>"
  "$<$<NOT:$<CONFIG:Debug>>:CFG_OTHER>"
  )
target_link_libraries(ConfigSourcesLinkIface ConfigSourcesIface)

# A target with sources in only one configuration that is not the
# first in CMAKE_CONFIGURATION_TYPES.
if(CMAKE_CONFIGURATION_TYPES MATCHES ";([^;]+)")
  set(one_config "${CMAKE_MATCH_1}")
else()
  set(one_config "${CMAKE_BUILD_TYPE}")
endif()
add_library(OneConfigOnly OBJECT "$<$<CONFIG:${one_config}>:${CMAKE_CURRENT_SOURCE_DIR}/iface_src.cpp>")
set_property(TARGET OneConfigOnly PROPERTY LINKER_LANGUAGE CXX)
