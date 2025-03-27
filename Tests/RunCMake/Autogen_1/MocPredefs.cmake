enable_language(CXX)

find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(MocPredefs MocPredefs.cxx)

if(NOT DEFINED CMAKE_CXX_COMPILER_PREDEFINES_COMMAND)
  return()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "LCC" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "1.26")
  return()
endif()

get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
  set(moc_predefs_h "moc_predefs_$<CONFIG>.h")
  set(check_dir "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
else()
  set(moc_predefs_h "moc_predefs.h")
  set(check_dir "${CMAKE_CURRENT_BINARY_DIR}")
endif()

add_custom_command(TARGET MocPredefs POST_BUILD VERBATIM COMMAND
  ${CMAKE_COMMAND}
    -Din=${CMAKE_CURRENT_BINARY_DIR}/MocPredefs_autogen/${moc_predefs_h}
    -Dout=${check_dir}/check_predefs.h
    -P${CMAKE_CURRENT_SOURCE_DIR}/MocPredefs-prefix.cmake
  )

add_executable(MocPredefs-check MocPredefs-check.cxx)
target_include_directories(MocPredefs-check PRIVATE ${check_dir})
add_dependencies(MocPredefs-check MocPredefs)

add_custom_target(MocPredefs-run-check ALL VERBATIM COMMAND MocPredefs-check)
