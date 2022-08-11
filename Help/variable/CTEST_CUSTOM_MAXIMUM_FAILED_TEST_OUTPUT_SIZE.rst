CTEST_CUSTOM_MAXIMUM_FAILED_TEST_OUTPUT_SIZE
--------------------------------------------

When saving a failing test's output, this is the maximum size, in bytes, that
will be collected by the :command:`ctest_test` command. Defaults to 307200
(300 KiB). See :variable:`CTEST_CUSTOM_TEST_OUTPUT_TRUNCATION` for possible
truncation modes.

If a test's output contains the literal string "CTEST_FULL_OUTPUT",
the output will not be truncated and may exceed the maximum size.

.. include:: CTEST_CUSTOM_XXX.txt

For controlling the output collection of passing tests, see
:variable:`CTEST_CUSTOM_MAXIMUM_PASSED_TEST_OUTPUT_SIZE`.
