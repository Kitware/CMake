cmake-test-launcher
-------------------

* A :variable:`CMAKE_TEST_LAUNCHER` variable and corresponding
  :prop_tgt:`TEST_LAUNCHER` target property were added to specify
  a launcher to be used by executable targets when invoked by
  tests added by the :command:`add_test` command.

* The :command:`add_test` command now honors
  :variable:`CMAKE_CROSSCOMPILING_EMULATOR` only when cross-compiling.
  See policy :policy:`CMP0158`.
