ExtractGTestMacro
-----------------

* A new :module:`GoogleTest` module was added to provide the
  :command:`gtest_add_tests` function independently of the :module:`FindGTest`
  module. The function was also updated to support keyword arguments, with
  functionality expanded to allow a test name prefix and suffix to be
  specified, the dependency on the source files to be optional and the list of
  discovered test cases to be returned to the caller.
