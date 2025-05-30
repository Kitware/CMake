gtest-use-json-output-for-discovery
-----------------------------------

* The :command:`gtest_discover_tests()` command from the :module:`GoogleTest`
  module now sets the ``DEF_SOURCE_LINE`` test property for each discovered
  test if gtest supports the ``--gtest_output=json`` option.  This test
  property is used by some IDEs to locate the source for each test.
* The :command:`gtest_discover_tests()` command from the :module:`GoogleTest`
  module previously parsed certain type-parameterized test names incorrectly.
  Their names ended up with raw characters from gtest's output and were
  very obviously misparsed.  Those names are now parsed correctly, so projects
  may see different test names to before for affected tests.
