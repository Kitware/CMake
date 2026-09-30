# FindOpenMP works only in a VS development environment.
set(CMake_TEST_FindOpenMP "OFF" CACHE BOOL "")

# Microsoft Assembler discovery works only in a VS development environment.
set(CMake_TEST_ASM_MASM "OFF" CACHE BOOL "")
set(CMake_TEST_ASM_MARMASM "OFF" CACHE BOOL "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_windows_clang_common.cmake")
