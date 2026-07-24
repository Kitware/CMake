ctest-test-label-and
---------------------

* The :command:`ctest_test` and :command:`ctest_memcheck` commands gained the
  ability to accept more than one value for their ``INCLUDE_LABEL`` and
  ``EXCLUDE_LABEL`` options.  When multiple regular expressions are given, a
  test is selected only if each expression matches at least one of the test's
  labels (i.e. the expressions form an ``AND`` relationship), matching the
  behavior of repeating the :option:`ctest --label-regex` and
  :option:`ctest --label-exclude` command-line options.
