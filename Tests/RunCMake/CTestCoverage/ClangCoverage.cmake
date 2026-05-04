enable_language(C)

if(NOT CMAKE_C_COMPILER_ID STREQUAL "Clang")
  message(FATAL_ERROR "This test requires a Clang C compiler.")
endif()

string(APPEND CMAKE_C_FLAGS " -fprofile-instr-generate -fcoverage-mapping")
set(CTEST_TEST_COVERAGE_TOOL "LLVM-COV")
include(CTest)

add_executable(ClangCoverage ClangCoverage.c)
add_test(NAME ClangCoverage COMMAND ClangCoverage)
