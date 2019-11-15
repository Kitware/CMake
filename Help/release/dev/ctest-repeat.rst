ctest-repeat
------------

* The :manual:`ctest(1)` tool gained a ``--repeat <mode>:<n>`` option
  to specify conditions in which to repeat tests.  This generalizes
  the existing ``--repeat-until-fail <n>`` option to add modes for
  ``until-pass`` and ``after-timeout``.

* The :command:`ctest_test` command gained a ``REPEAT <mode>:<n>`` option
  to specify conditions in which to repeat tests.
