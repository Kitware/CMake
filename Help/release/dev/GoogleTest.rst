GoogleTest
----------

* The :module:`GoogleTest` module gained a new command
  :command:`gtest_discover_tests` implementing dynamic (build-time) test
  discovery.  Unlike the source parsing approach, dynamic discovery executes
  the test (in 'list available tests' mode) at build time to discover tests.
  This is robust against unusual ways of labeling tests, provides much better
  support for advanced features such as parameterized tests, and does not
  require re-running CMake to discover added or removed tests within a test
  executable.
