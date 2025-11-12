cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_library NONE)

find_package(export_sepdir REQUIRED)

if (NOT TARGET CXXModules::export_sepdir)
  message(FATAL_ERROR
    "Missing imported target")
endif ()

get_property(modules TARGET CXXModules::export_sepdir
  PROPERTY "IMPORTED_CXX_MODULES_NOCONFIG")
if (modules STREQUAL "")
  message(SEND_ERROR
    "Expected non-empty property value for IMPORTED_CXX_MODULES_NOCONFIG")
endif ()
