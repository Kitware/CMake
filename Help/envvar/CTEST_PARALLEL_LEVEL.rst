CTEST_PARALLEL_LEVEL
--------------------

.. include:: ENV_VAR.txt

Specify the number of tests for CTest to run in parallel.
For example, if ``CTEST_PARALLEL_LEVEL`` is set to 8, CTest will run
up to 8 tests concurrently as if ``ctest`` were invoked with the
:option:`--parallel 8 <ctest --parallel>` option.

See :manual:`ctest(1)` for more information on parallel test execution.
