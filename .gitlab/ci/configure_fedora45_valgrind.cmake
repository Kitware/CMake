set(CMake_TEST_Qt5 ON CACHE BOOL "")
set(CMake_TEST_Qt6 ON CACHE BOOL "")

# Disable bootstrap testing for Valgrind testing.
set(CMake_TEST_BOOTSTRAP FALSE CACHE BOOL "")

# Reduce debug info for Valgrind to load by using -g1 instead of -g.
set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g1 -DNDEBUG" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g1 -DNDEBUG" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_fedora45_ninja_common.cmake")
