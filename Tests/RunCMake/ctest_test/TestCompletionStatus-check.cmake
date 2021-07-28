file(READ "${RunCMake_TEST_BINARY_DIR}/Testing/TAG" _tag)
string(REGEX REPLACE "^([^\n]*)\n.*$" "\\1" _date "${_tag}")
file(READ "${RunCMake_TEST_BINARY_DIR}/Testing/${_date}/Test.xml" _test_contents)

# Check custom completion status.
if(NOT _test_contents MATCHES [[<Value>CustomDetails</Value>]])
  string(APPEND RunCMake_TEST_FAILED
    "Could not find expected <Value>CustomDetails</Value> in Test.xml")
endif()
# Check test output.
if(NOT _test_contents MATCHES "test output")
  string(APPEND RunCMake_TEST_FAILED "Could not find expected string 'test output' in Test.xml")
endif()
if(NOT _test_contents MATCHES "more output")
  string(APPEND RunCMake_TEST_FAILED "Could not find expected string 'more output' in Test.xml")
endif()
