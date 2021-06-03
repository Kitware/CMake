set(BUILD_CursesDialog ON CACHE BOOL "")
set(BUILD_QtDialog ON CACHE BOOL "")
set(CMake_QT_MAJOR_VERSION "5" CACHE STRING "")
set(CMake_TEST_JQ "/usr/bin/jq" CACHE PATH "")
set(CMake_TEST_JSON_SCHEMA ON CACHE BOOL "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_common.cmake")
