set(BUILD_QtDialog ON CACHE BOOL "")
set(BUILD_CursesDialog ON CACHE BOOL "")
set(CMAKE_PREFIX_PATH "$ENV{CI_PROJECT_DIR}/.gitlab/qt" CACHE STRING "")
set(CMake_TEST_Java OFF CACHE BOOL "")
set(Python_FIND_REGISTRY NEVER CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_common.cmake")
