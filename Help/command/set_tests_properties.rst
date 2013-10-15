set_tests_properties
--------------------

Set a property of the tests.

::

  set_tests_properties(test1 [test2...] PROPERTIES prop1 value1 prop2 value2)

Set a property for the tests.  If the property is not found, CMake
will report an error.  Generator expressions will be expanded the same
as supported by the test's add_test call.  The properties include:

WILL_FAIL: If set to true, this will invert the pass/fail flag of the
test.

PASS_REGULAR_EXPRESSION: If set, the test output will be checked
against the specified regular expressions and at least one of the
regular expressions has to match, otherwise the test will fail.

::

  Example: PASS_REGULAR_EXPRESSION "TestPassed;All ok"

FAIL_REGULAR_EXPRESSION: If set, if the output will match to one of
specified regular expressions, the test will fail.

::

  Example: PASS_REGULAR_EXPRESSION "[^a-z]Error;ERROR;Failed"

Both PASS_REGULAR_EXPRESSION and FAIL_REGULAR_EXPRESSION expect a list
of regular expressions.

TIMEOUT: Setting this will limit the test runtime to the number of
seconds specified.
