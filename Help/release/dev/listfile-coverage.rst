listfile-coverage
-----------------

* The :envvar:`CMAKE_LISTFILE_COVERAGE_DIR` environment variable was added
  to record coverage of CMake language code in lcov tracefiles.

* :manual:`ctest(1)` gained a ``CMAKE`` value for its
  :ref:`CoverageTool <ctest-CoverageTool>` setting to record coverage of
  CMake language code for each test.

* The :command:`ctest_coverage` command learned to read coverage of CMake
  language code recorded by the ``CMAKE`` coverage tool.
