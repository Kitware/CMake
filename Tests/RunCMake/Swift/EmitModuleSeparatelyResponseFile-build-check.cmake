if(NOT actual_stdout MATCHES
    "swiftc(\\.exe)?\"? [^\n]* -c @CMakeFiles(/|\\\\)L\\.dir(/|\\\\)(Debug(/|\\\\))?L\\.o(bj)?\\.swift\\.rsp")
  string(APPEND RunCMake_TEST_FAILED
    "No Swift compile response-file command found for target L.\n")
endif()

if(NOT actual_stdout MATCHES
    "swiftc(\\.exe)?\"? [^\n]* -emit-module @.*L\\.swiftmodule(/|\\\\)[-_a-zA-Z0-9]+\\.swiftmodule\\.rsp")
  string(APPEND RunCMake_TEST_FAILED
    "No Swift emit-module response-file command found for target L.\n")
endif()
