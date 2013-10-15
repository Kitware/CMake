ctest_test
----------

Run tests in the project build tree.

::

  ctest_test([BUILD build_dir] [APPEND]
             [START start number] [END end number]
             [STRIDE stride number] [EXCLUDE exclude regex ]
             [INCLUDE include regex] [RETURN_VALUE res]
             [EXCLUDE_LABEL exclude regex]
             [INCLUDE_LABEL label regex]
             [PARALLEL_LEVEL level]
             [SCHEDULE_RANDOM on]
             [STOP_TIME time of day])

Tests the given build directory and stores results in Test.xml.  The
second argument is a variable that will hold value.  Optionally, you
can specify the starting test number START, the ending test number
END, the number of tests to skip between each test STRIDE, a regular
expression for tests to run INCLUDE, or a regular expression for tests
to not run EXCLUDE.  EXCLUDE_LABEL and INCLUDE_LABEL are regular
expression for test to be included or excluded by the test property
LABEL.  PARALLEL_LEVEL should be set to a positive number representing
the number of tests to be run in parallel.  SCHEDULE_RANDOM will
launch tests in a random order, and is typically used to detect
implicit test dependencies.  STOP_TIME is the time of day at which the
tests should all stop running.

The APPEND option marks results for append to those previously
submitted to a dashboard server since the last ctest_start.  Append
semantics are defined by the dashboard server in use.
