cmake_minimum_required(VERSION 3.23...3.24)

if(DEFINED include_before_project)
  include("${include_before_project}")
endif()

project(${RunCMake_TEST} NONE)

if(DEFINED include_after_project)
  include("${include_after_project}")
endif()

include(${RunCMake_TEST}.cmake OPTIONAL)
