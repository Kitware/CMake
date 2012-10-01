foreach (_retval 0 1)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ret${_retval}.c" "int main(){return ${_retval};}\n")
endforeach ()

# create binaries that we will use as a pseudo memory checker
add_executable(pseudo_valgrind "${CMAKE_CURRENT_BINARY_DIR}/ret0.c")
set_target_properties(pseudo_valgrind PROPERTIES OUTPUT_NAME valgrind)
add_executable(pseudo_purify "${CMAKE_CURRENT_BINARY_DIR}/ret0.c")
set_target_properties(pseudo_purify PROPERTIES OUTPUT_NAME purify)
add_executable(pseudo_BC "${CMAKE_CURRENT_BINARY_DIR}/ret0.c")
set_target_properties(pseudo_BC PROPERTIES OUTPUT_NAME BC)

# binary to be used as pre- and post-memcheck command that fails
add_executable(memcheck_fail "${CMAKE_CURRENT_BINARY_DIR}/ret1.c")

foreach  (_test IN ITEMS Unknown NotExist DummyValgrind DummyValgrindPrePost
        DummyValgrindFailPre DummyValgrindFailPost DummyPurify DummyBC
        DummyValgrindIgnoreMemcheck)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/${_test}/test.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${_test}/test.cmake"
        @ONLY ESCAPE_QUOTES)
    add_test(NAME CTestTestMemcheck${_test}
        COMMAND ${CMAKE_CTEST_COMMAND}
        -S "${CMAKE_CURRENT_BINARY_DIR}/${_test}/test.cmake" -V
        --output-log "${CMAKE_CURRENT_BINARY_DIR}/${_test}/testOutput.log"
        -D PSEUDO_BC=$<TARGET_FILE:pseudo_BC>
        -D PSEUDO_PURIFY=$<TARGET_FILE:pseudo_purify>
        -D PSEUDO_VALGRIND=$<TARGET_FILE:pseudo_valgrind>
        -D ERROR_COMMAND=$<TARGET_FILE:memcheck_fail>
        )
endforeach ()

file(TO_NATIVE_PATH "${CMAKE_COMMAND}" CMAKE_COMMAND_NATIVE)
string(REPLACE "\\" "\\\\" CMAKE_COMMAND_NATIVE "${CMAKE_COMMAND_NATIVE}")
string(REPLACE "(" "\\(" CMAKE_COMMAND_NATIVE "${CMAKE_COMMAND_NATIVE}")
string(REPLACE ")" "\\)" CMAKE_COMMAND_NATIVE "${CMAKE_COMMAND_NATIVE}")
string(REPLACE "+" "\\+" CMAKE_COMMAND_NATIVE "${CMAKE_COMMAND_NATIVE}")
set_tests_properties(CTestTestMemcheckUnknown PROPERTIES
    PASS_REGULAR_EXPRESSION "Do not understand memory checker: ${CMAKE_COMMAND_NATIVE}\n(.*\n)?Error in read script: ${CMAKE_CURRENT_BINARY_DIR}/Unknown/test.cmake\n")

set_tests_properties(CTestTestMemcheckNotExist PROPERTIES
    PASS_REGULAR_EXPRESSION "Memory checker \\(MemoryCheckCommand\\) not set, or cannot find the specified program.")

# It is a valid result if valgrind does not output any files (can e.g. happen
# if you have not compiled in debug mode), so these tests will not fail.
set_tests_properties(CTestTestMemcheckDummyValgrind CTestTestMemcheckDummyValgrindPrePost
    PROPERTIES
    PASS_REGULAR_EXPRESSION "\n-- Processing memory checking output: \nMemory checking results:\n$")

set_tests_properties(CTestTestMemcheckDummyValgrindFailPre
    PROPERTIES
    PASS_REGULAR_EXPRESSION "\nProblem running command: ${CMAKE_CURRENT_BINARY_DIR}[^\n]*fail[^\n]*\n(.*\n)?Problem executing pre-memcheck command\\(s\\\).\n(.*\n)?Error in read script: ${CMAKE_CURRENT_BINARY_DIR}/DummyValgrindFailPre/test.cmake\n")

set_tests_properties(CTestTestMemcheckDummyValgrindFailPost
    PROPERTIES
    PASS_REGULAR_EXPRESSION "\nProblem running command: ${CMAKE_CURRENT_BINARY_DIR}[^\n]*fail[^\n]*\n(.*\n)?Problem executing post-memcheck command\\(s\\\).\n(.*\n)?Error in read script: ${CMAKE_CURRENT_BINARY_DIR}/DummyValgrindFailPost/test.cmake\n")

set_tests_properties(CTestTestMemcheckDummyValgrindIgnoreMemcheck
    PROPERTIES
    PASS_REGULAR_EXPRESSION "\n2/2 Test #2: RunCMakeAgain .*\n1/1 MemCheck #1: RunCMake .*\n-- Processing memory checking output: \nMemory checking results:\n$")

set_tests_properties(CTestTestMemcheckDummyPurify PROPERTIES
    PASS_REGULAR_EXPRESSION "\nCannot find memory tester output file: ${CMAKE_CURRENT_BINARY_DIR}/DummyPurify/Testing/Temporary/MemoryChecker.log\n(.*\n)?Error in read script: ${CMAKE_CURRENT_BINARY_DIR}/DummyPurify/test.cmake\n")

set_tests_properties(CTestTestMemcheckDummyBC PROPERTIES
    PASS_REGULAR_EXPRESSION "\nCannot find memory tester output file: ${CMAKE_CURRENT_BINARY_DIR}/DummyBC/Testing/Temporary/MemoryChecker.log\n(.*\n)?Error parsing XML in stream at line 1: no element found\n(.*\n)?Error in read script: ${CMAKE_CURRENT_BINARY_DIR}/DummyBC/test.cmake\n")
