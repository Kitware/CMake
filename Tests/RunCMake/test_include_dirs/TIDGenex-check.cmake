# Assert the serialization of the generated CTestTestfile.cmake for the
# TEST_INCLUDE_FILE(S) generator-expression feature.
set(ctf "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake")
if(NOT EXISTS "${ctf}")
  set(RunCMake_TEST_FAILED "CTestTestfile.cmake not found:\n  ${ctf}")
  return()
endif()
file(READ "${ctf}" content)

# Plain (singular) entry stays a bare, double-quoted include (regression).
if(NOT content MATCHES "include\\(\"[^\n]*plain[.]cmake\"\\)")
  string(APPEND RunCMake_TEST_FAILED
    "plain include not emitted as a bare double-quoted include\n")
endif()

# Config-independent genex collapses to one unconditional bare include.
if(NOT content MATCHES "include\\(\"[^\n]*indep[.]cmake\"\\)")
  string(APPEND RunCMake_TEST_FAILED
    "config-independent genex not collapsed to one unconditional include\n")
endif()

# A conditional-empty genex must never emit an empty include().
if(content MATCHES "include\\(\"\"\\)")
  string(APPEND RunCMake_TEST_FAILED "empty include() emitted\n")
endif()

if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  if(NOT content MATCHES "CTEST_CONFIGURATION_TYPE MATCHES \"\\^\\(\\[Dd\\]\\[Ee\\]\\[Bb\\]\\[Uu\\]\\[Gg\\]\\)[$]\"")
    string(APPEND RunCMake_TEST_FAILED
      "expected per-config Debug guard not found on multi-config\n")
  endif()
  if(NOT content MATCHES "include\\(\"[^\n]*props_Release[.]cmake\"\\)")
    string(APPEND RunCMake_TEST_FAILED
      "expected props_Release.cmake per-config branch not found\n")
  endif()
else()
  if(NOT content MATCHES "include\\(\"[^\n]*props_Debug[.]cmake\"\\)")
    string(APPEND RunCMake_TEST_FAILED
      "expected unconditional props_Debug.cmake include not found\n")
  endif()
  if(NOT content MATCHES "include\\(\"[^\n]*dbgonly[.]cmake\"\\)")
    string(APPEND RunCMake_TEST_FAILED
      "expected unconditional dbgonly.cmake include not found\n")
  endif()
  if(content MATCHES "CTEST_CONFIGURATION_TYPE MATCHES")
    string(APPEND RunCMake_TEST_FAILED
      "unexpected per-config guard on single-config generator\n")
  endif()
endif()
