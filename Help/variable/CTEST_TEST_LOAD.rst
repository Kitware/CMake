CTEST_TEST_LOAD
------------------

Specify the ``TestLoad`` setting
in a :manual:`ctest(1)` dashboard client script.

This creates a CPU load threshold that CTest will attempt to
not cross.  CTest will not start a new test if doing so
would be likely to raise the CPU load above this threshold.
Please note that this variable is only honored during parallel testing
(``ctest -j`` or similar).

Here are the various ways that you can set this threshold:

* The ``TEST_LOAD`` option to :command:`ctest_test`.
* :variable:`CTEST_TEST_LOAD` in a dashboard script.
* Passing the ``--ctest-load`` command-line argument to :manual:`ctest(1)`.
* Setting the :variable:`CMAKE_TEST_LOAD` variable in your CMake project.

These are listed here in descending priority order, ie
``ctest_test(TEST_LOAD ...)`` will override any of the other
methods of setting this threshold.  Also note that
:variable:`CMAKE_TEST_LOAD` is not honored when running ctest
in script (``-S``) mode.
