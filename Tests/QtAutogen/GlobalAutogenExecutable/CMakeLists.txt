cmake_minimum_required(VERSION 3.26)
project(GlobalAutogenExecutable)

include("../AutogenCoreTest.cmake")

set(test_automoc_path "global_automoc_exe_path")
set(test_autouic_path "global_autouic_exe_path")
set(test_autorcc_path "global_autorcc_exe_path")

set(CMAKE_AUTOMOC_EXECUTABLE ${test_automoc_path})
set(CMAKE_AUTOUIC_EXECUTABLE ${test_autouic_path})
set(CMAKE_AUTORCC_EXECUTABLE ${test_autorcc_path})

add_executable(autogen_test main.cpp)

get_target_property(target_automoc_path autogen_test AUTOMOC_EXECUTABLE)
get_target_property(target_autouic_path autogen_test AUTOUIC_EXECUTABLE)
get_target_property(target_autorcc_path autogen_test AUTORCC_EXECUTABLE)

if(NOT ${target_automoc_path} STREQUAL ${test_automoc_path})
  message(FATAL_ERROR "CMAKE_AUTOMOC_EXECUTABLE not set")
endif()

if (NOT ${target_autouic_path} STREQUAL ${test_autouic_path})
  message(FATAL_ERROR "CMAKE_AUTOUIC_EXECUTABLE not set")
endif()

if (NOT ${target_autorcc_path} STREQUAL ${test_autorcc_path})
  message(FATAL_ERROR "CMAKE_AUTORCC_EXECUTABLE not set")
endif()
