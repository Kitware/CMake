cmake_minimum_required(VERSION 3.24)
project(cxx_modules_library NONE)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "ac01f462-0f5f-432a-86aa-acef252918a6")

find_package(export_interfaces_no_properties REQUIRED)

if (NOT TARGET CXXModules::export_interfaces_no_properties)
  message(FATAL_ERROR
    "Missing imported target")
endif ()

get_property(file_sets TARGET CXXModules::export_interfaces_no_properties
  PROPERTY INTERFACE_CXX_MODULE_SETS)
if (NOT file_sets STREQUAL "modules")
  message(FATAL_ERROR
    "Incorrect exported file sets in CXXModules::export_interfaces_no_properties:\n  ${file_sets}")
endif ()

get_property(file_set_files TARGET CXXModules::export_interfaces_no_properties
  PROPERTY CXX_MODULE_SET_modules)
set(expected_file_set_files
  "${expected_dir}/importable.cxx"
  "${expected_dir}/subdir/importable.cxx"
  )
if (NOT file_set_files STREQUAL "${expected_file_set_files}")
  message(FATAL_ERROR
    "Incorrect exported file set paths in CXXModules::export_interfaces_no_properties:\n  ${file_set_files}")
endif ()

get_property(imported_modules_set TARGET CXXModules::export_interfaces_no_properties
  PROPERTY IMPORTED_CXX_MODULES_DEBUG SET)
if (imported_modules_set)
  message(FATAL_ERROR
    "Unexpected C++ modules specified.")
endif ()
