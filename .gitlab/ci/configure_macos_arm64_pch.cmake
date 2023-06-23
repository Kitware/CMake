set(CMake_BUILD_PCH "ON" CACHE BOOL "")

# sccache does not forward the PCH '-Xarch_arm64 "-include/..."' flag correctly.
set(configure_no_sccache 1)

include("${CMAKE_CURRENT_LIST_DIR}/configure_macos_common.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/configure_common.cmake")
