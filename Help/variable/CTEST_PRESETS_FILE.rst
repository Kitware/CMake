CTEST_PRESETS_FILE
------------------

.. versionadded:: 4.4

Specify the ``PRESETS_FILE`` option for all relevant :manual:`ctest(1)`
:ref:`Dashboard Client` step commands in a dashboard script,
or on the :program:`ctest` command line via the :ctest-dashboard-option:`-D` option.

This variable sets the ``PRESETS_FILE`` option for
:command:`ctest_configure`, :command:`ctest_build`, :command:`ctest_test`,
and :command:`ctest_memcheck` when those commands are not given an explicit
``PRESETS_FILE`` argument.

See also :variable:`CTEST_PRESET`, :variable:`CTEST_CONFIGURE_PRESET`,
:variable:`CTEST_BUILD_PRESET`, and :variable:`CTEST_TEST_PRESET`.
