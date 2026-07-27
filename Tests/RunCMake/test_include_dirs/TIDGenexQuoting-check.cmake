set(ctf "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake")
if(NOT EXISTS "${ctf}")
  set(RunCMake_TEST_FAILED "CTestTestfile.cmake not found:\n  ${ctf}")
  return()
endif()
file(READ "${ctf}" content)

# (1) ';' result: split into two separate double-quoted includes (fan-out),
#     not one combined include.
if(NOT content MATCHES "include\\(\"[^\n]*/a\"\\)")
  string(APPEND RunCMake_TEST_FAILED
    "semicolon result first element not emitted as its own include\n")
endif()
if(NOT content MATCHES "include\\(\"b[.]cmake\"\\)")
  string(APPEND RunCMake_TEST_FAILED
    "semicolon result second element not emitted as its own include\n")
endif()
if(content MATCHES "include\\(\"[^\n]*a;b[.]cmake\"\\)")
  string(APPEND RunCMake_TEST_FAILED
    "semicolon result was not split into multiple includes\n")
endif()

# (2) space result: double-quoted.
if(NOT content MATCHES "include\\(\"[^\n]*a b[.]cmake\"\\)")
  string(APPEND RunCMake_TEST_FAILED "space result not double-quoted\n")
endif()

# (3) '$' result: bracket form, and NOT double-quoted.
if(NOT content MATCHES "include\\(\\[=*\\[[^\n]*x[$]y[.]cmake[^\n]*\\]=*\\]\\)")
  string(APPEND RunCMake_TEST_FAILED "dollar result not emitted in bracket form\n")
endif()
if(content MATCHES "include\\(\"[^\n]*x[$]y[.]cmake")
  string(APPEND RunCMake_TEST_FAILED
    "dollar result was double-quoted (should be bracketed)\n")
endif()

# (4) plain ${VAR} entry preserved raw and bare (double-quoted, not bracketed).
if(NOT content MATCHES "include\\(\"[^\n]*lit_[^\n]*MY_VAR[^\n]*[.]cmake\"\\)")
  string(APPEND RunCMake_TEST_FAILED "plain \${VAR} entry not preserved raw\n")
endif()
if(content MATCHES "include\\(\\[=*\\[[^\n]*MY_VAR")
  string(APPEND RunCMake_TEST_FAILED
    "plain \${VAR} entry was bracketed (should stay raw double-quoted)\n")
endif()
