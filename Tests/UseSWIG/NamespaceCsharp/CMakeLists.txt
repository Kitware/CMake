cmake_minimum_required(VERSION 3.12...3.13)

project(TestNamespaceCsharp CXX)

include(CTest)

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

set(UseSWIG_MODULE_VERSION 2)


add_library(ns_example STATIC ns_example.cpp)
target_include_directories(ns_example PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

set_property(SOURCE ns_example.i PROPERTY CPLUSPLUS ON)

swig_add_library(ns_csharp TYPE SHARED LANGUAGE csharp SOURCES ns_example.i)
set_target_properties(ns_csharp PROPERTIES SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE)

target_link_libraries(ns_csharp PRIVATE ns_example)

get_target_property(NS_CSHARP_SUPPORT_FILES_DIR ns_csharp SWIG_SUPPORT_FILES_DIRECTORY)

add_test(NAME NamespaceCsharp COMMAND "${CMAKE_COMMAND}" "-DSUPPORT_FILES_DIRECTORY=${NS_CSHARP_SUPPORT_FILES_DIR}" -P "${CMAKE_CURRENT_SOURCE_DIR}/ValidateSupportFiles.cmake")
