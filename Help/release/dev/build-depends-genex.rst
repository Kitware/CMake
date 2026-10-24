build-depends-genex
-------------------

* The ``BUILD_DEPENDS`` options of the :command:`add_test`,
  :command:`discover_tests` and :command:`install` commands, and the
  :prop_dir:`CMAKE_TEST_BUILD_DEPENDS` directory property, now support
  :manual:`generator expressions <cmake-generator-expressions(7)>`.
  Targets referenced by an expression, such as ``$<TARGET_FILE:tgt>``,
  become dependencies, and entries evaluating to an empty string are ignored.
