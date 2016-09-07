test-fixtures
-------------

* CTest now supports test fixtures through the new :prop_test:`FIXTURES_SETUP`,
  :prop_test:`FIXTURES_CLEANUP` and :prop_test:`FIXTURES_REQUIRED` test
  properties. When using regular expressions or ``--rerun-failed`` to limit
  the tests to be run, a fixture's setup and cleanup tests will automatically
  be added to the execution set if any test requires that fixture.
