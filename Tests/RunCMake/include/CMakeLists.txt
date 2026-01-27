set(root_test OFF)
if(RunCMake_TEST STREQUAL "ParentVariableRoot" OR
   RunCMake_TEST STREQUAL "CMP0198-NEW-root" OR
   RunCMake_TEST STREQUAL "CMP0198-implicit-NEW-root" OR
   RunCMake_TEST STREQUAL "CMP0198-OLD-root" OR
   RunCMake_TEST STREQUAL "CMP0198-WARN-root")
  set(root_test ON)
endif()

if(root_test)
  message(STATUS "CMakeLists\.txt: '${CMAKE_PARENT_LIST_FILE}'")
endif()

if(RunCMake_TEST STREQUAL "CMP0198-implicit-NEW-root")
  cmake_minimum_required(VERSION 4.1...4.2)
else()
  cmake_minimum_required(VERSION 3.10)
endif()

project(${RunCMake_TEST} NONE)

if(root_test)
  message(STATUS "CMakeLists\.txt: '${CMAKE_PARENT_LIST_FILE}'")
  include(ParentVariableRoot/include1.cmake)
  message(STATUS "CMakeLists\.txt: '${CMAKE_PARENT_LIST_FILE}'")
  return()
endif()

include(${RunCMake_TEST}.cmake)
