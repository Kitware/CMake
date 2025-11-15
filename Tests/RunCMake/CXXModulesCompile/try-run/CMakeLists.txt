cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_try_run CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

try_run(can_run_modules_result can_compile_modules
  SOURCES_TYPE CXX_MODULE
  SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/importable.cxx"
  SOURCES_TYPE NORMAL
  SOURCE_FROM_FILE
    main.cxx "${CMAKE_CURRENT_LIST_DIR}/main.cxx"
  CXX_STANDARD 20)

if (NOT can_compile_modules)
  message(FATAL_ERROR
    "`try_run` could not compile sources using modules.")
endif ()

if (can_run_modules_result)
  message(FATAL_ERROR
    "`try_run` could not run sources using modules.")
endif ()
