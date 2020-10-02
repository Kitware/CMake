cmake_minimum_required (VERSION 3.18...3.19)

find_package (SWIG 1.0)
if (NOT SWIG_FOUND)
  message (FATAL_ERROR "Failed to find SWIG with version 1.0")
endif()
