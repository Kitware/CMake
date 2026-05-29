CTEST_PRESET
------------

.. versionadded:: 4.4

Specify a preset name to use for all relevant :manual:`ctest(1)`
:ref:`Dashboard Client` steps in a dashboard script,
or on the :program:`ctest` command line via the :ctest-dashboard-option:`-D` option.

This variable sets the ``PRESET`` option for :command:`ctest_configure`,
:command:`ctest_build`, :command:`ctest_test`, and :command:`ctest_memcheck`
when those commands are not given an explicit ``PRESET`` argument and the
corresponding command-specific variable
(:variable:`CTEST_CONFIGURE_PRESET`, :variable:`CTEST_BUILD_PRESET`, or
:variable:`CTEST_TEST_PRESET`) is also unset.

For :command:`ctest_configure`, an error is raised if no configure preset
with this name exists.  For :command:`ctest_build`, :command:`ctest_test`,
and :command:`ctest_memcheck`, a warning is emitted and the variable is
ignored if no preset of the matching type exists with this name.

See also :variable:`CTEST_PRESETS_FILE`.
