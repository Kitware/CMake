ctest-test-load-option
----------------------

* CTest learned to optionally measure the CPU load during parallel
  testing and avoid starting tests that may cause the load to exceed
  a given threshold.  See the :manual:`ctest(1)` command ``--test-load``
  option, the ``TestLoad`` setting of the :ref:`CTest Test Step`,
  the :variable:`CTEST_TEST_LOAD` variable, and the ``TEST_LOAD``
  option of the :command:`ctest_test` command.
