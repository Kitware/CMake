make-test-depend-on-all
-----------------------

* The :variable:`CMAKE_SKIP_TEST_ALL_DEPENDENCY` variable was added
  to control whether the ``test`` (or ``RUN_TESTS``) buildsystem
  target depends on the ``all`` (or ``ALL_BUILD``) target.
