cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_library NONE)

find_package(export_interfaces REQUIRED)

if (NOT TARGET CXXModules::export_interfaces)
  message(FATAL_ERROR
    "Missing imported target")
endif ()

get_property(file_sets TARGET CXXModules::export_interfaces
  PROPERTY INTERFACE_CXX_MODULE_SETS)
if (NOT file_sets STREQUAL "modules")
  message(FATAL_ERROR
    "Incorrect exported file sets in CXXModules::export_interfaces:\n  ${file_sets}")
endif ()

get_property(file_set_files TARGET CXXModules::export_interfaces
  PROPERTY CXX_MODULE_SET_modules)
set(expected_file_set_files
  "${expected_source_dir}/importable.cxx"
  "${expected_source_dir}/subdir/importable.cxx"
  )
if (NOT file_set_files STREQUAL "${expected_file_set_files}")
  message(FATAL_ERROR
    "Incorrect exported file set paths in CXXModules::export_interfaces:\n  ${file_set_files}")
endif ()

get_property(imported_modules TARGET CXXModules::export_interfaces
  PROPERTY IMPORTED_CXX_MODULES_DEBUG)
set(expected_imported_modules
  "importable=${expected_source_dir}/importable.cxx"
  "subdir_importable=${expected_source_dir}/subdir/importable.cxx"
  )
if (NOT imported_modules STREQUAL "${expected_imported_modules}")
  message(FATAL_ERROR
    "Incorrect exported modules in CXXModules::export_interfaces:\n"
    "  ${imported_modules}\n"
    "does not match:\n"
    "  ${expected_imported_modules}"
  )
endif ()
