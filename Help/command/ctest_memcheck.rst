ctest_memcheck
--------------

Run tests with a dynamic analysis tool.

::

  ctest_memcheck([BUILD build_dir] [RETURN_VALUE res] [APPEND]
             [START start number] [END end number]
             [STRIDE stride number] [EXCLUDE exclude regex ]
             [INCLUDE include regex]
             [EXCLUDE_LABEL exclude regex]
             [INCLUDE_LABEL label regex]
             [PARALLEL_LEVEL level] )

Tests the given build directory and stores results in MemCheck.xml.
The second argument is a variable that will hold value.  Optionally,
you can specify the starting test number START, the ending test number
END, the number of tests to skip between each test STRIDE, a regular
expression for tests to run INCLUDE, or a regular expression for tests
not to run EXCLUDE.  EXCLUDE_LABEL and INCLUDE_LABEL are regular
expressions for tests to be included or excluded by the test property
LABEL.  PARALLEL_LEVEL should be set to a positive number representing
the number of tests to be run in parallel.

The APPEND option marks results for append to those previously
submitted to a dashboard server since the last ctest_start.  Append
semantics are defined by the dashboard server in use.
