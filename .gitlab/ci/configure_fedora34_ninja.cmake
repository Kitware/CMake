set(CMake_TEST_ISPC "ON" CACHE STRING "")
set(CMake_TEST_GUI "ON" CACHE BOOL "")

# Cover compilation with C++11 only and not higher standards.
set(CMAKE_CXX_STANDARD "11" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_fedora34_common.cmake")
