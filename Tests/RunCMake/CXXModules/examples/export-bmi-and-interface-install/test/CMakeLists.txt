cmake_minimum_required(VERSION 3.24)
project(cxx_modules_library NONE)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "9629ab6c-6c0e-423f-bb9d-cc5ac4a22041")

find_package(export_bmi_and_interfaces REQUIRED)

if (NOT TARGET CXXModules::export_bmi_and_interfaces)
  message(FATAL_ERROR
    "Missing imported target")
endif ()

get_property(file_sets TARGET CXXModules::export_bmi_and_interfaces
  PROPERTY INTERFACE_CXX_MODULE_SETS)
if (NOT file_sets STREQUAL "modules")
  message(FATAL_ERROR
    "Incorrect exported file sets in `CXXModules::export_bmi_and_interfaces`: `${file_sets}`")
endif ()

get_property(file_set_files TARGET CXXModules::export_bmi_and_interfaces
  PROPERTY CXX_MODULE_SET_modules)
if (NOT file_set_files STREQUAL "${expected_source_dir}/importable.cxx")
  message(FATAL_ERROR
    "Incorrect exported file set paths in CXXModules::export_bmi_and_interfaces`: `${file_set_files}`")
endif ()

get_property(imported_modules TARGET CXXModules::export_bmi_and_interfaces
  PROPERTY IMPORTED_CXX_MODULES_DEBUG)
if (NOT imported_modules MATCHES "importable=${expected_source_dir}/importable.cxx,${expected_binary_dir}/importable.(gcm|pcm|ifc)")
  message(FATAL_ERROR
    "Incorrect exported modules in CXXModules::export_bmi_and_interfaces`: `${imported_modules}`")
endif ()
