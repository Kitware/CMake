set(CMake_RUN_IWYU ON CACHE BOOL "")
set(CMake_IWYU_OPTIONS "-DCMAKE_IWYU_FORWARD_STD_HASH" CACHE STRING "")
# Uncomment to diagnose IWYU problems as needed.
#set(CMake_IWYU_VERBOSE ON CACHE BOOL "")
set(IWYU_COMMAND "/usr/bin/include-what-you-use-15" CACHE FILEPATH "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_common.cmake")
