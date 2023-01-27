set(CMake_RUN_IWYU ON CACHE BOOL "")
# Uncomment to diagnose IWYU problems as needed.
#set(CMake_IWYU_VERBOSE ON CACHE BOOL "")
set(IWYU_COMMAND "/usr/bin/include-what-you-use-6.0" CACHE FILEPATH "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_common.cmake")
