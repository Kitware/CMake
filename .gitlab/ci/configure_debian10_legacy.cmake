set(CMake_TEST_FindPython2 "ON" CACHE BOOL "")
set(CMake_TEST_FindPython2_IronPython "ON" CACHE BOOL "")
set(CMake_TEST_FindPython2_NumPy "ON" CACHE BOOL "")
set(CMake_TEST_FindPython2_PyPy "ON" CACHE BOOL "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
