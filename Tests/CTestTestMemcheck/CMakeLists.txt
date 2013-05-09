foreach (_retval 0 1)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/memtester.cxx.in" "${CMAKE_CURRENT_BINARY_DIR}/ret${_retval}.cxx" @ONLY)
endforeach ()

include_directories(${CMake_SOURCE_DIR}/Source ${CMake_BINARY_DIR}/Source)

# create binaries that we will use as a pseudo memory checker
add_executable(pseudo_valgrind "${CMAKE_CURRENT_BINARY_DIR}/ret0.cxx")
set_target_properties(pseudo_valgrind PROPERTIES OUTPUT_NAME valgrind)
target_link_libraries(pseudo_valgrind CMakeLib)

# Xcode 2.x forgets to create the output directory before linking
# the individual architectures.
if(CMAKE_OSX_ARCHITECTURES AND XCODE AND NOT "${XCODE_VERSION}" MATCHES "^[^12]")
    add_custom_command(TARGET pseudo_valgrind
                       PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CFG_INTDIR}"
    )
endif()

add_executable(pseudo_purify "${CMAKE_CURRENT_BINARY_DIR}/ret0.cxx")
set_target_properties(pseudo_purify PROPERTIES OUTPUT_NAME purify)
target_link_libraries(pseudo_purify CMakeLib)
add_executable(pseudo_BC "${CMAKE_CURRENT_BINARY_DIR}/ret0.cxx")
set_target_properties(pseudo_BC PROPERTIES OUTPUT_NAME BC)
target_link_libraries(pseudo_BC CMakeLib)

# binary to be used as pre- and post-memcheck command that fails
add_executable(memcheck_fail "${CMAKE_CURRENT_BINARY_DIR}/ret1.cxx")
target_link_libraries(memcheck_fail CMakeLib)

# Binaries that are used as memchecker that do not write the expected
# output file. Need to be in their own subdirectory as they have the
# same filenames.
add_subdirectory(NoLogDummyChecker)

set(NORMAL_CTEST_OUTPUT "\n1/1 MemCheck #1: RunCMake \\.+   Passed +[0-9]+.[0-9]+ sec\n\n100% tests passed, 0 tests failed out of 1\n.*\n-- Processing memory checking output: \nMemory checking results:\n")
set(BULLSEYE_MSG "(BullseyeCoverage[^\n]*\n)?")

function(gen_mc_test_internal NAME CHECKER)
    # just to clean old directories from the time these had real source dirs
    file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}")
    # end

    set(SUBTEST_NAME "${NAME}")
    set(CHECKER_COMMAND "${CHECKER}")
    foreach(_file IN ITEMS CMakeLists.txt CTestConfig.cmake test.cmake)
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${_file}.in"
            "${CMAKE_CURRENT_BINARY_DIR}/${NAME}/${_file}" @ONLY)
    endforeach()
    add_test(NAME CTestTestMemcheck${NAME}
        COMMAND ${CMAKE_CTEST_COMMAND}
        -C $<CONFIGURATION>
        -S "${CMAKE_CURRENT_BINARY_DIR}/${NAME}/test.cmake" -V
        --output-log "${CMAKE_CURRENT_BINARY_DIR}/${NAME}/testOutput.log"
        ${ARGN}
        )
endfunction(gen_mc_test_internal)

function(gen_mc_test NAME CHECKER)
    gen_mc_test_internal(${NAME} "${CHECKER}"
        -D PSEUDO_BC=$<TARGET_FILE:pseudo_BC>
        -D PSEUDO_PURIFY=$<TARGET_FILE:pseudo_purify>
        -D PSEUDO_VALGRIND=$<TARGET_FILE:pseudo_valgrind>
        -D ERROR_COMMAND=$<TARGET_FILE:memcheck_fail>
        )
endfunction(gen_mc_test)

function(gen_mcnl_test NAME CHECKER)
    gen_mc_test_internal(${NAME} ${CHECKER}
        -D PSEUDO_BC=$<TARGET_FILE:pseudonl_BC>
        -D PSEUDO_PURIFY=$<TARGET_FILE:pseudonl_purify>
        -D PSEUDO_VALGRIND=$<TARGET_FILE:pseudonl_valgrind>
        )
    set_tests_properties(CTestTestMemcheck${NAME}
        PROPERTIES
        PASS_REGULAR_EXPRESSION "\nCannot find memory tester output file: ${CMAKE_CURRENT_BINARY_DIR}/${NAME}/Testing/Temporary/MemoryChecker.log\n(.*\n)?Error in read script: ${CMAKE_CURRENT_BINARY_DIR}/${NAME}/test.cmake\n")
endfunction(gen_mcnl_test)

unset(CTEST_EXTRA_CONFIG)
unset(CTEST_EXTRA_CODE)
unset(CMAKELISTS_EXTRA_CODE)

gen_mc_test(DummyPurify "\${PSEUDO_PURIFY}")
gen_mc_test(DummyValgrind "\${PSEUDO_VALGRIND}")
gen_mc_test(DummyBC "\${PSEUDO_BC}")
gen_mcnl_test(DummyPurifyNoLogfile "\${PSEUDO_PURIFY}")
gen_mcnl_test(DummyValgrindNoLogfile "\${PSEUDO_VALGRIND}")
gen_mcnl_test(DummyBCNoLogfile "\${PSEUDO_BC}")

set(CTEST_EXTRA_CODE "set(CTEST_CUSTOM_PRE_MEMCHECK \"\${CTEST_MEMORYCHECK_COMMAND}\")\nset(CTEST_CUSTOM_POST_MEMCHECK \"\${CTEST_MEMORYCHECK_COMMAND}\")")
gen_mc_test(DummyValgrindPrePost "\${PSEUDO_VALGRIND}")

set(CTEST_EXTRA_CODE "set(CTEST_CUSTOM_POST_MEMCHECK \"\${ERROR_COMMAND}\")")
gen_mc_test(DummyValgrindFailPost "\${PSEUDO_VALGRIND}")

set(CTEST_EXTRA_CODE "set(CTEST_CUSTOM_PRE_MEMCHECK \"\${ERROR_COMMAND}\")")
gen_mc_test(DummyValgrindFailPre "\${PSEUDO_VALGRIND}")

unset(CTEST_EXTRA_CODE)
set(CTEST_EXTRA_CONFIG "set(CTEST_CUSTOM_MEMCHECK_IGNORE RunCMakeAgain)\n")
set(CMAKELISTS_EXTRA_CODE "add_test(NAME RunCMakeAgain COMMAND \"\${CMAKE_COMMAND}\" --version)")
gen_mc_test(DummyValgrindIgnoreMemcheck "\${PSEUDO_VALGRIND}")

set(CTEST_EXTRA_CONFIG "set(CTEST_MEMORYCHECK_SUPPRESSIONS_FILE \"\${CMAKE_CURRENT_BINARY_DIR}/does-not-exist\")")
unset(CMAKELISTS_EXTRA_CODE)
gen_mc_test(DummyValgrindInvalidSupFile "\${PSEUDO_VALGRIND}")

# CTest will add the logfile option as last option. Tell the dummy memcheck
# to ignore that argument. This will cause the logfile to be missing, which
# will be the prove for us that the custom option is indeed used.
set(CTEST_EXTRA_CONFIG "set(CTEST_MEMORYCHECK_COMMAND_OPTIONS \"--\")")
gen_mc_test(DummyValgrindCustomOptions "\${PSEUDO_VALGRIND}")

unset(CTEST_EXTRA_CONFIG)
gen_mc_test(NotExist "\${CTEST_BINARY_DIRECTORY}/no-memcheck-exe")

gen_mc_test(Unknown "${CMAKE_COMMAND}")

string(REPLACE "\\" "\\\\" CMAKE_COMMAND_ESCAPED "${CMAKE_COMMAND}")
string(REPLACE "(" "\\(" CMAKE_COMMAND_ESCAPED "${CMAKE_COMMAND_ESCAPED}")
string(REPLACE ")" "\\)" CMAKE_COMMAND_ESCAPED "${CMAKE_COMMAND_ESCAPED}")
string(REPLACE "+" "\\+" CMAKE_COMMAND_ESCAPED "${CMAKE_COMMAND_ESCAPED}")

set_tests_properties(CTestTestMemcheckUnknown PROPERTIES
    PASS_REGULAR_EXPRESSION "Do not understand memory checker: ${CMAKE_COMMAND_ESCAPED}\n(.*\n)?Error in read script: ${CMAKE_CURRENT_BINARY_DIR}/Unknown/test.cmake\n")

set_tests_properties(CTestTestMemcheckNotExist PROPERTIES
    PASS_REGULAR_EXPRESSION "Memory checker \\(MemoryCheckCommand\\) not set, or cannot find the specified program.")

# It is a valid result if valgrind does not output any files (can e.g. happen
# if you have not compiled in debug mode), so these tests will not fail.
set_tests_properties(CTestTestMemcheckDummyValgrind
                     CTestTestMemcheckDummyValgrindPrePost
                     CTestTestMemcheckDummyPurify
    PROPERTIES
    PASS_REGULAR_EXPRESSION "${NORMAL_CTEST_OUTPUT}${BULLSEYE_MSG}$")

foreach (_pp Pre Post)
    string(TOLOWER ${_pp} _pp_lower)
    set_tests_properties(CTestTestMemcheckDummyValgrindFail${_pp}
        PROPERTIES
        PASS_REGULAR_EXPRESSION "\nProblem running command: ${CMAKE_CURRENT_BINARY_DIR}[^\n]*fail[^\n]*\n(.*\n)?Problem executing ${_pp_lower}-memcheck command\\(s\\\).\n(.*\n)?Error in read script: ${CMAKE_CURRENT_BINARY_DIR}/DummyValgrindFail${_pp}/test.cmake\n")
endforeach ()

set_tests_properties(CTestTestMemcheckDummyValgrindIgnoreMemcheck
    PROPERTIES
    PASS_REGULAR_EXPRESSION "\n2/2 Test #2: RunCMakeAgain .*${NORMAL_CTEST_OUTPUT}${BULLSEYE_MSG}$")

set_tests_properties(CTestTestMemcheckDummyBC PROPERTIES
    PASS_REGULAR_EXPRESSION "\n1/1 MemCheck #1: RunCMake \\.+   Passed +[0-0]+.[0-9]+ sec\n\n100% tests passed, 0 tests failed out of 1\n(.*\n)?Error parsing XML in stream at line 1: no element found\n")

set_tests_properties(CTestTestMemcheckDummyValgrindInvalidSupFile PROPERTIES
    PASS_REGULAR_EXPRESSION "\nCannot find memory checker suppression file: ${CMAKE_CURRENT_BINARY_DIR}/does-not-exist\n")

set_tests_properties(CTestTestMemcheckDummyValgrindCustomOptions PROPERTIES
    PASS_REGULAR_EXPRESSION "\nCannot find memory tester output file: ${CMAKE_CURRENT_BINARY_DIR}/DummyValgrindCustomOptions/Testing/Temporary/MemoryChecker.log\n(.*\n)?Error in read script: ${CMAKE_CURRENT_BINARY_DIR}/DummyValgrindCustomOptions/test.cmake\n")
