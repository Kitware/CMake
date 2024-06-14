cmake_minimum_required(VERSION 3.28)
project(cxx_modules_library NONE)

find_package(export_transitive_targets REQUIRED)

if (NOT TARGET CXXModules::export_transitive_targets)
  message(FATAL_ERROR
    "Missing imported target")
endif ()

get_property(transitive_targets TARGET CXXModules::export_transitive_targets
  PROPERTY IMPORTED_CXX_MODULES_LINK_LIBRARIES)
if (NOT transitive_targets STREQUAL "CXXModules::usage_reqs")
  message(FATAL_ERROR
    "Incorrect transitive link library in C++ module interface CXXModules::export_transitive_targets:\n  ${transitive_targets}")
endif ()
