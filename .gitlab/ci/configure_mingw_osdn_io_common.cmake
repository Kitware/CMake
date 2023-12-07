set(CMake_TEST_Java OFF CACHE BOOL "")

get_filename_component(mingw_dir "${CMAKE_CURRENT_LIST_DIR}/../mingw" ABSOLUTE)
set(CMake_TEST_MSYSTEM_PREFIX "${mingw_dir}" CACHE STRING "")

set(configure_no_sccache 1)

include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
