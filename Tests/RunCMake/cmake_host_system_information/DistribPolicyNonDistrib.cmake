# CMP0221 unset (WARN): a non-DISTRIB_* query must not warn, even with a
# non-empty CMAKE_SYSROOT.  Also proves the os-release is not read lazily when
# no DISTRIB_* key is requested (no stderr, no side effects).
set(CMAKE_SYSROOT "${CMAKE_CURRENT_LIST_DIR}/Sentinel")
cmake_host_system_information(RESULT cores QUERY NUMBER_OF_LOGICAL_CORES)
