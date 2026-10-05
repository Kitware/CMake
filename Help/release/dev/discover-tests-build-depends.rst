discover-tests-build-depends
----------------------------

* The :command:`discover_tests` command gained a ``BUILD_DEPENDS`` argument to
  support the :ctest-option:`--out-of-date` behavior of :manual:`ctest(1)`.

* The :prop_dir:`CMAKE_TEST_BUILD_DEPENDS` directory property was added to
  support directory-scoped build time dependencies for tests.
