FindGTest-AUTO-SOURCES
----------------------

* The :module:`FindGTest` module ``gtest_add_tests`` macro learned
  a new ``AUTO`` option to automatically read the :prop_tgt:`SOURCES`
  target property of the test executable and scan the source files
  for tests to be added.
