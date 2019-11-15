ctest-repeat-until-pass
-----------------------

* The :manual:`ctest(1)` tool learned new ``--repeat-until-pass <n>``
  and ``--repeat-after-timeout <n>`` options to help tolerate sporadic
  test failures.

* The :command:`ctest_test` command gained a ``REPEAT <mode>:<n>`` option
  to specify conditions in which to repeat tests.
