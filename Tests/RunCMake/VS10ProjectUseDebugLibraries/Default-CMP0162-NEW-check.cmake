include(${CMAKE_CURRENT_LIST_DIR}/check-common.cmake)

UseDebugLibraries_check(default "true" "false")
UseDebugLibraries_check(defaultCLR "true" "false")
UseDebugLibraries_check(defaultUtil "true" "false")
UseDebugLibraries_check(defaultRTL "false" "false")
UseDebugLibraries_check(ALL_BUILD "true" "false")
UseDebugLibraries_check(ZERO_CHECK "true" "false")
