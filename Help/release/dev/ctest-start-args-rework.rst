ctest-start-args-rework
-----------------------

* The :command:`ctest_start` command has been reworked so that you can simply
  call ``ctest_start(APPEND)`` and it will read all the needed information from
  the TAG file. The argument parsing has also been relaxed so that the order of
  the arguments is less significant.
