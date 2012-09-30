foreach (_retval 0 1)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ret${_retval}.c" "int main(){return ${_retval};}\n")
endforeach ()

# create binaries that we will use as a pseudo memory checker
add_executable(valgrind "${CMAKE_CURRENT_BINARY_DIR}/ret0.c")
add_executable(purify "${CMAKE_CURRENT_BINARY_DIR}/ret0.c")
add_executable(BC "${CMAKE_CURRENT_BINARY_DIR}/ret0.c")

# binary to be used as pre- and post-memcheck command that fails
add_executable(fail "${CMAKE_CURRENT_BINARY_DIR}/ret1.c")

foreach  (_test IN ITEMS Unknown NotExist DummyValgrind DummyValgrindPrePost
        DummyValgrindFailPre DummyValgrindFailPost DummyPurify DummyBC)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/${_test}/test.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${_test}/test.cmake"
        @ONLY ESCAPE_QUOTES)
    add_test(CTestTestMemcheck${_test} ${CMAKE_CTEST_COMMAND}
        -S "${CMAKE_CURRENT_BINARY_DIR}/${_test}/test.cmake" -V
        --output-log "${CMAKE_CURRENT_BINARY_DIR}/${_test}/testOutput.log"
        )
endforeach ()

file(TO_NATIVE_PATH "${CMAKE_COMMAND}" CMAKE_COMMAND_NATIVE)
string(REPLACE "\\" "\\\\" CMAKE_COMMAND_NATIVE "{CMAKE_COMMAND_NATIVE}")
set_tests_properties(CTestTestMemcheckUnknown PROPERTIES
    PASS_REGULAR_EXPRESSION "Do not understand memory checker: ${CMAKE_COMMAND}\n.*Error in read script: ${CMAKE_CURRENT_BINARY_DIR}/Unknown/test.cmake\n$")

set_tests_properties(CTestTestMemcheckNotExist PROPERTIES
    PASS_REGULAR_EXPRESSION "Memory checker \\(MemoryCheckCommand\\) not set, or cannot find the specified program.")

# It is a valid result if valgrind does not output any files (can e.g. happen
# if you have not compiled in debug mode), so these tests will not fail.
set_tests_properties(CTestTestMemcheckDummyValgrind CTestTestMemcheckDummyValgrindPrePost
    PROPERTIES
    PASS_REGULAR_EXPRESSION "\n-- Processing memory checking output: \nMemory checking results:\n$")

set_tests_properties(CTestTestMemcheckDummyValgrindFailPre
    PROPERTIES
    PASS_REGULAR_EXPRESSION "\nProblem running command: ${CMAKE_CURRENT_BINARY_DIR}[^\n]*fail[^\n]*\n(.*\n)?Problem executing pre-memcheck command\\(s\\\).\n(.*\n)?Error in read script: ${CMAKE_CURRENT_BINARY_DIR}/DummyValgrindFailPre/test.cmake\n$")

set_tests_properties(CTestTestMemcheckDummyValgrindFailPost
    PROPERTIES
    PASS_REGULAR_EXPRESSION "\nProblem running command: ${CMAKE_CURRENT_BINARY_DIR}[^\n]*fail[^\n]*\n(.*\n)?Problem executing post-memcheck command\\(s\\\).\n(.*\n)?Error in read script: ${CMAKE_CURRENT_BINARY_DIR}/DummyValgrindFailPost/test.cmake\n$")

set_tests_properties(CTestTestMemcheckDummyPurify PROPERTIES
    PASS_REGULAR_EXPRESSION "\nCannot find memory tester output file: ${CMAKE_CURRENT_BINARY_DIR}/DummyPurify/Testing/Temporary/MemoryChecker.log\n(.*\n)?Error in read script: ${CMAKE_CURRENT_BINARY_DIR}/DummyPurify/test.cmake\n$")

set_tests_properties(CTestTestMemcheckDummyBC PROPERTIES
    PASS_REGULAR_EXPRESSION "\nCannot find memory tester output file: ${CMAKE_CURRENT_BINARY_DIR}/DummyBC/Testing/Temporary/MemoryChecker.log\n(.*\n)?Error parsing XML in stream at line 1: no element found\n(.*\n)?Error in read script: ${CMAKE_CURRENT_BINARY_DIR}/DummyBC/test.cmake\n$")
