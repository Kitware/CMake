CTEST_NIGHTLY_START_TIME
------------------------

.. versionadded:: 3.1

Specify the CTest ``NightlyStartTime`` setting in a :manual:`ctest(1)`
:ref:`Dashboard Client` script,
or on the :program:`ctest` command line via the :ctest-dashboard-option:`-D` option.

Note that this variable must always be set for a nightly build in a
dashboard script. It is needed so that nightly builds can be properly grouped
together in CDash.
