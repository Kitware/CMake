excludeFixtures
---------------

* The :manual:`ctest(1)` executable gained new options which allow the
  developer to disable automatically adding tests to the test set to satisfy
  fixture dependencies. ``-FS`` prevents adding setup tests for fixtures
  matching the provided regular expression, ``-FC`` prevents adding cleanup
  tests for matching fixtures and ``-FA`` prevents adding any test for matching
  fixtures.
