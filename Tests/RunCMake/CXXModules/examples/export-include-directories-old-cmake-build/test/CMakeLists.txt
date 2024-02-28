cmake_minimum_required(VERSION 3.22)
project(cxx_modules_library NONE)

set(CMAKE_VERSION "3.22.0") # Trigger non-fileset export code.
find_package(export_include_directories_old_cmake REQUIRED)

if (NOT TARGET CXXModules::export_include_directories_old_cmake)
  message(FATAL_ERROR
    "Missing imported target")
endif ()

get_property(include_directories TARGET CXXModules::export_include_directories_old_cmake
  PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
if (include_directories)
  message(FATAL_ERROR
    "Exported include directory on C++ module interface CXXModules::export_include_directories_old_cmake:\n  ${include_directories}")
endif ()
