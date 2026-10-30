CTEST_TIME_LIMIT
----------------

Specify the CTest ``TimeLimit`` setting
in a :manual:`ctest(1)` :ref:`Dashboard Client` script,
or on the :program:`ctest` command line via the :ctest-dashboard-option:`-D` option.

The value is the maximum time, in seconds, allowed for the whole
dashboard run, measured from the start of :program:`ctest`.
The :command:`ctest_build` and :command:`ctest_coverage` commands do
nothing if less than two minutes remain.  The :command:`ctest_test` and
:command:`ctest_memcheck` commands limit the timeout of each test so
that it ends at least two minutes before the time limit.
