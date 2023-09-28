cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_library NONE)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "ac01f462-0f5f-432a-86aa-acef252918a6")

find_package(export_include_directories REQUIRED)

if (NOT TARGET CXXModules::export_include_directories)
  message(FATAL_ERROR
    "Missing imported target")
endif ()

get_property(file_sets TARGET CXXModules::export_include_directories
  PROPERTY INTERFACE_CXX_MODULE_SETS)
if (NOT file_sets STREQUAL "modules")
  message(FATAL_ERROR
    "Incorrect exported file sets in CXXModules::export_include_directories:\n  ${file_sets}")
endif ()
