# The information file is what a later configure reloads, so load it the same
# way and check that every field detection computed was recorded.  The caller
# sets the expect_* values for the assembler flavor under test.
include("${RunCMake_TEST_BINARY_DIR}/CMakeFiles/${CMAKE_VERSION}/CMakeASMCompiler.cmake")

foreach(var
    CMAKE_ASM_COMPILER_ID
    CMAKE_ASM_SIMULATE_ID
    CMAKE_ASM_COMPILER_FRONTEND_VARIANT
    )
  if(NOT "${${var}}" STREQUAL "${expect_${var}}")
    string(APPEND RunCMake_TEST_FAILED
      "${var} was not recorded in the assembler information file:\n"
      "  expected: '${expect_${var}}'\n"
      "  reloaded: '${${var}}'\n")
  endif()
endforeach()
