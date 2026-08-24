# Shared check for the OutputSync argv/probe cases.  The calling test sets
# expectation variables (all optional) before each build command:
#   expect_otarget            0/1  -- "-Otarget" must be absent/present
#   expect_otarget_count      N    -- exact number of "-Otarget" occurrences
#   expect_uses_terminal_flag 0/1  -- "CMAKE_USES_TERMINAL_PREFIX=+" absent/present
#   expect_probe              0/1  -- the make tool must not/must be probed
#   expect_probe_count        N    -- exact number of "--version" probes
#   expect_order_onone        0/1  -- "-Otarget" must appear before "-Onone"

set(record "${RunCMake_TEST_BINARY_DIR}/fake_make_record.txt")
set(marker "${RunCMake_TEST_BINARY_DIR}/fake_make_probe.txt")

if(EXISTS "${record}")
  file(READ "${record}" record_content)
else()
  set(record_content "")
endif()

if(EXISTS "${marker}")
  file(READ "${marker}" marker_content)
else()
  set(marker_content "")
endif()

string(REGEX MATCHALL "-Otarget" _otarget_matches "${record_content}")
list(LENGTH _otarget_matches _otarget_count)

string(REGEX MATCHALL "probe" _probe_matches "${marker_content}")
list(LENGTH _probe_matches _probe_count)

if(DEFINED expect_otarget)
  if(expect_otarget AND _otarget_count EQUAL 0)
    string(APPEND RunCMake_TEST_FAILED
      "Expected '-Otarget' in the build command but recorded:\n${record_content}\n")
  elseif(NOT expect_otarget AND _otarget_count GREATER 0)
    string(APPEND RunCMake_TEST_FAILED
      "Did not expect '-Otarget' in the build command but recorded:\n${record_content}\n")
  endif()
endif()

if(DEFINED expect_otarget_count AND NOT _otarget_count EQUAL expect_otarget_count)
  string(APPEND RunCMake_TEST_FAILED
    "Expected ${expect_otarget_count} '-Otarget' occurrence(s) but found ${_otarget_count}:\n${record_content}\n")
endif()

if(DEFINED expect_uses_terminal_flag)
  string(FIND "${record_content}" "CMAKE_USES_TERMINAL_PREFIX=+" _ut_pos)
  if(expect_uses_terminal_flag AND _ut_pos EQUAL -1)
    string(APPEND RunCMake_TEST_FAILED
      "Expected 'CMAKE_USES_TERMINAL_PREFIX=+' in the build command but recorded:\n${record_content}\n")
  elseif(NOT expect_uses_terminal_flag AND NOT _ut_pos EQUAL -1)
    string(APPEND RunCMake_TEST_FAILED
      "Did not expect 'CMAKE_USES_TERMINAL_PREFIX=+' in the build command but recorded:\n${record_content}\n")
  endif()
endif()

if(DEFINED expect_probe)
  if(expect_probe AND _probe_count EQUAL 0)
    string(APPEND RunCMake_TEST_FAILED
      "Expected the make tool to be probed with '--version' but it was not.\n")
  elseif(NOT expect_probe AND _probe_count GREATER 0)
    string(APPEND RunCMake_TEST_FAILED
      "Did not expect a '--version' probe but it ran ${_probe_count} time(s).\n")
  endif()
endif()

if(DEFINED expect_probe_count AND NOT _probe_count EQUAL expect_probe_count)
  string(APPEND RunCMake_TEST_FAILED
    "Expected ${expect_probe_count} probe(s) but found ${_probe_count}.\n")
endif()

if(DEFINED expect_order_onone AND expect_order_onone)
  if(NOT record_content MATCHES "-Otarget[^\n]*-Onone")
    string(APPEND RunCMake_TEST_FAILED
      "Expected '-Otarget' to appear before '-Onone' but recorded:\n${record_content}\n")
  endif()
endif()
