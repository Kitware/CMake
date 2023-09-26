cmake_minimum_required(VERSION 3.24)
project(cxx_modules_library NONE)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "ac01f462-0f5f-432a-86aa-acef252918a6")

find_package(export_include_directories REQUIRED)

if (NOT TARGET CXXModules::export_include_directories)
  message(FATAL_ERROR
    "Missing imported target")
endif ()

get_property(include_directories TARGET CXXModules::export_include_directories
  PROPERTY IMPORTED_CXX_MODULES_INCLUDE_DIRECTORIES)
foreach (include_directory IN LISTS include_directories)
  if (NOT EXISTS "${include_directory}")
    message(FATAL_ERROR
      "Missing include directory in C++ module interface CXXModules::export_include_directories:\n  ${include_directory}")
  endif ()
endforeach ()
