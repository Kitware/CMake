cmake_minimum_required(VERSION 3.20)
project(DashboardExample CXX)

include(CTest)

add_executable(passes passes.cxx)
add_test(NAME passes COMMAND passes)

add_executable(fails fails.cxx)
add_test(NAME fails COMMAND fails)

# Optionally add a target that fails to compile, to show how build errors
# are reported on the dashboard.
option(DASHBOARD_EXAMPLE_BUILD_ERROR "Add a target that fails to compile" OFF)
if(DASHBOARD_EXAMPLE_BUILD_ERROR)
  add_executable(error error.cxx)
endif()
