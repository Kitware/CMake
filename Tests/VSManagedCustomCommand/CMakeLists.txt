cmake_minimum_required(VERSION 3.20)
project(VSManagedCustomCommand CXX)

add_custom_command(OUTPUT middle.txt
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/example.txt ${CMAKE_CURRENT_BINARY_DIR}/middle.txt
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/example.txt
  )

add_custom_command(OUTPUT example.cxx
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/middle.txt ${CMAKE_CURRENT_BINARY_DIR}/example.cxx
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/middle.txt
  )

add_library(example SHARED example.cxx)
set_property(TARGET example PROPERTY COMMON_LANGUAGE_RUNTIME "")
