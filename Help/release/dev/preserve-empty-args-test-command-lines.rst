preserve-empty-args-test-command-lines
--------------------------------------

* Empty list elements in the :prop_tgt:`TEST_LAUNCHER` and
  :prop_tgt:`CROSSCOMPILING_EMULATOR` target properties are now preserved
  when the executable for a command given to :command:`add_test` is a CMake
  target. See policy :policy:`CMP0178`.

* Empty list elements in the :prop_tgt:`TEST_LAUNCHER` and
  :prop_tgt:`CROSSCOMPILING_EMULATOR` target properties are now preserved
  for the test created by :command:`ExternalData_Add_Test` from the
  :module:`ExternalData` module.  See policy :policy:`CMP0178`.

* Empty list elements in the :prop_tgt:`TEST_LAUNCHER` and
  :prop_tgt:`CROSSCOMPILING_EMULATOR` target properties are now preserved
  for tests created by :command:`gtest_add_tests` and
  :command:`gtest_discover_tests` from the :module:`GoogleTest` module.
  Empty list elements after the ``EXTRA_ARGS`` keyword of these two commands
  are also now preserved.  See policy :policy:`CMP0178`.
