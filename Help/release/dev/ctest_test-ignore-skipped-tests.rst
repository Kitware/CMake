ctest_test-ignore-skipped-tests
-------------------------------

* When running tests, CTest learned to treat skipped tests (using the
  :prop_test:`SKIP_RETURN_CODE` property) the same as tests with the
  :prop_test:`DISABLED` property. Due to this change, CTest will not indicate
  failure when all tests are either skipped or pass.
