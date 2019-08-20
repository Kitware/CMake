file(READ ${RunCMake_SOURCE_DIR}/trace-stderr.txt expected_content)
string(REGEX REPLACE "\n+$" "" expected_content "${expected_content}")

file(READ ${RunCMake_BINARY_DIR}/redirected.trace actual_content)
string(REGEX REPLACE "\r\n" "\n" actual_content "${actual_content}")
string(REGEX REPLACE "\n+$" "" actual_content "${actual_content}")
if(NOT "${actual_content}" MATCHES "${expected_content}")
    set(RunCMake_TEST_FAILED
        "Trace file content does not match that expected."
        "Expected to match:\n${expected_content}\n"
        "Actual content:\n${actual_content}\n"
        )
endif()
