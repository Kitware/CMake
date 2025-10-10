cmake_minimum_required(VERSION 3.28)
project(cxx_modules_library NONE)

find_package(export_with_headers REQUIRED)

if (NOT TARGET CXXModules::export_with_headers)
  message(FATAL_ERROR
    "Missing imported target")
endif ()

get_property(iface_includes TARGET CXXModules::export_with_headers
  PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
set(include_dir "${CMAKE_CURRENT_SOURCE_DIR}")
cmake_path(GET include_dir PARENT_PATH include_dir)
string(APPEND include_dir "/include")
if (NOT iface_includes STREQUAL "${include_dir};$<BUILD_INTERFACE:${include_dir}>")
  message(FATAL_ERROR
    "Incorrect include interface for CXXModules::export_with_headers:\n  ${iface_includes}")
endif ()
