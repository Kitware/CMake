cmake_minimum_required(VERSION 3.28)
project(cxx_modules_library NONE)

find_package(export_transitive_modules REQUIRED)

if (NOT TARGET CXXModules::export_transitive_modules1)
  message(FATAL_ERROR
    "Missing transitive imported target")
endif ()

if (NOT TARGET CXXModules::export_transitive_modules)
  message(FATAL_ERROR
    "Missing imported target")
endif ()
