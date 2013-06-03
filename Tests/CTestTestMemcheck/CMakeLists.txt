REGEX_ESCAPE_STRING(CTEST_ESCAPED_CMAKE_CURRENT_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

foreach (_retval 0 1)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ret${_retval}.c" "int main(){return ${_retval};}\n")
endforeach ()

# create binaries that we will use as a pseudo memory checker
add_executable(pseudo_valgrind "${CMAKE_CURRENT_BINARY_DIR}/ret0.c")
set_target_properties(pseudo_valgrind PROPERTIES OUTPUT_NAME valgrind)

# Xcode 2.x forgets to create the output directory before linking
# the individual architectures.
if(CMAKE_OSX_ARCHITECTURES AND XCODE AND NOT "${XCODE_VERSION}" MATCHES "^[^12]")
    add_custom_command(TARGET pseudo_valgrind
                       PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CFG_INTDIR}"
    )
endif()

add_executable(pseudo_purify "${CMAKE_CURRENT_BINARY_DIR}/ret0.c")
set_target_properties(pseudo_purify PROPERTIES OUTPUT_NAME purify)
add_executable(pseudo_BC "${CMAKE_CURRENT_BINARY_DIR}/ret0.c")
set_target_properties(pseudo_BC PROPERTIES OUTPUT_NAME BC)

# binary to be used as pre- and post-memcheck command that fails
add_executable(memcheck_fail "${CMAKE_CURRENT_BINARY_DIR}/ret1.c")

foreach  (_test IN ITEMS Unknown UnknownQuoted NotExist
        DummyValgrind DummyValgrindPrePost
        DummyValgrindFailPre DummyValgrindFailPost
        DummyPurify DummyBC
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

string(REPLACE "\\" "\\\\" CMAKE_COMMAND_ESCAPED "${CMAKE_COMMAND}")
string(REPLACE "(" "\\(" CMAKE_COMMAND_ESCAPED "${CMAKE_COMMAND_ESCAPED}")
string(REPLACE ")" "\\)" CMAKE_COMMAND_ESCAPED "${CMAKE_COMMAND_ESCAPED}")
string(REPLACE "+" "\\+" CMAKE_COMMAND_ESCAPED "${CMAKE_COMMAND_ESCAPED}")

foreach (_unkn Unknown UnknownQuoted)
    set_tests_properties(CTestTestMemcheck${_unkn} PROPERTIES
        PASS_REGULAR_EXPRESSION "Do not understand memory checker: ${CMAKE_COMMAND_ESCAPED}\n(.*\n)?Error in read script: ${CTEST_ESCAPED_CMAKE_CURRENT_BINARY_DIR}/${_unkn}/test.cmake\n")
endforeach ()

set_tests_properties(CTestTestMemcheckNotExist PROPERTIES
    PASS_REGULAR_EXPRESSION "Memory checker \\(MemoryCheckCommand\\) not set, or cannot find the specified program.")

set(NORMAL_CTEST_OUTPUT "\n-- Processing memory checking output: \nMemory checking results:\n(BullseyeCoverage[^\n]*\n)?")

# It is a valid result if valgrind does not output any files (can e.g. happen
# if you have not compiled in debug mode), so these tests will not fail.
set_tests_properties(CTestTestMemcheckDummyValgrind CTestTestMemcheckDummyValgrindPrePost
    PROPERTIES
    PASS_REGULAR_EXPRESSION "${NORMAL_CTEST_OUTPUT}")

foreach (_pp Pre Post)
    string(TOLOWER ${_pp} _pp_lower)
    set_tests_properties(CTestTestMemcheckDummyValgrindFail${_pp}
        PROPERTIES
        PASS_REGULAR_EXPRESSION "\nProblem running command: ${CTEST_ESCAPED_CMAKE_CURRENT_BINARY_DIR}[^\n]*fail[^\n]*\n(.*\n)?Problem executing ${_pp_lower}-memcheck command\\(s\\\).\n(.*\n)?Error in read script: ${CTEST_ESCAPED_CMAKE_CURRENT_BINARY_DIR}/DummyValgrindFail${_pp}/test.cmake\n")
endforeach ()

set_tests_properties(CTestTestMemcheckDummyValgrindIgnoreMemcheck
    PROPERTIES
    PASS_REGULAR_EXPRESSION "\n2/2 Test #2: RunCMakeAgain .*\n1/1 MemCheck #1: RunCMake .*${NORMAL_CTEST_OUTPUT}")

set_tests_properties(CTestTestMemcheckDummyPurify PROPERTIES
    PASS_REGULAR_EXPRESSION "\nCannot find memory tester output file: ${CTEST_ESCAPED_CMAKE_CURRENT_BINARY_DIR}/DummyPurify/Testing/Temporary/MemoryChecker.log\n(.*\n)?Error in read script: ${CTEST_ESCAPED_CMAKE_CURRENT_BINARY_DIR}/DummyPurify/test.cmake\n")

set_tests_properties(CTestTestMemcheckDummyBC PROPERTIES
    PASS_REGULAR_EXPRESSION "\nCannot find memory tester output file: ${CTEST_ESCAPED_CMAKE_CURRENT_BINARY_DIR}/DummyBC/Testing/Temporary/MemoryChecker.log\n(.*\n)?Error parsing XML in stream at line 1: no element found\n(.*\n)?Error in read script: ${CTEST_ESCAPED_CMAKE_CURRENT_BINARY_DIR}/DummyBC/test.cmake\n")
