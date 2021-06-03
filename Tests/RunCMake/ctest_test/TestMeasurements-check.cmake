file(READ "${RunCMake_TEST_BINARY_DIR}/Testing/TAG" _tag)
string(REGEX REPLACE "^([^\n]*)\n.*$" "\\1" _date "${_tag}")
file(READ "${RunCMake_TEST_BINARY_DIR}/Testing/${_date}/Test.xml" _test_contents)

# Check double measurement.
if(NOT _test_contents MATCHES [[NamedMeasurement type="numeric/double" name="my_custom_value"]])
  string(APPEND RunCMake_TEST_FAILED
    "Could not find expected <NamedMeasurement> tag for type='numeric/double' in Test.xml")
endif()
if(NOT _test_contents MATCHES "<Value>1.4847</Value>")
  string(APPEND RunCMake_TEST_FAILED "Could not find expected measurement value in Test.xml")
endif()
# Check img measurement.
if(NOT _test_contents MATCHES [[NamedMeasurement name="TestImage" type="image/png" encoding="base64"]])
  string(APPEND RunCMake_TEST_FAILED
    "Could not find expected <NamedMeasurement> tag for type='image/png' in Test.xml")
endif()
