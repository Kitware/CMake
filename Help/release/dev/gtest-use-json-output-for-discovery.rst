gtest-use-json-output-for-discovery
-----------------------------------

* The :command:`gtest_discover_tests()` command from the :module:`GoogleTest`
  module now sets the ``DEF_SOURCE_LINE`` test property for each discovered
  test if gtest supports the ``--gtest_output=json`` option.  This test
  property is used by some IDEs to locate the source for each test.
