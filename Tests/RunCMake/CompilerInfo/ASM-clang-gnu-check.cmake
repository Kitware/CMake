# A GNU-like command-line leaves the simulate id empty.
set(expect_CMAKE_ASM_COMPILER_ID "Clang")
set(expect_CMAKE_ASM_SIMULATE_ID "")
set(expect_CMAKE_ASM_COMPILER_FRONTEND_VARIANT "GNU")
include("${RunCMake_SOURCE_DIR}/ASM-check-common.cmake")
