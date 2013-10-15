ctest_coverage
--------------

Collect coverage tool results.

::

  ctest_coverage([BUILD build_dir] [RETURN_VALUE res] [APPEND]
                 [LABELS label1 [label2 [...]]])

Perform the coverage of the given build directory and stores results
in Coverage.xml.  The second argument is a variable that will hold
value.

The LABELS option filters the coverage report to include only source
files labeled with at least one of the labels specified.

The APPEND option marks results for append to those previously
submitted to a dashboard server since the last ctest_start.  Append
semantics are defined by the dashboard server in use.
