set(CMake_TEST_WIX_NO_VERIFY "ON" CACHE BOOL "")
set(CMake_TEST_GUI "ON" CACHE BOOL "")
set(CMake_TEST_FindOpenGL "ON" CACHE BOOL "")
set(CMake_TEST_IPO_WORKS_C "ON" CACHE BOOL "")
set(CMake_TEST_IPO_WORKS_CXX "ON" CACHE BOOL "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_windows_common.cmake")
