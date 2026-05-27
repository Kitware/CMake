ctest-script-preset-arg
------------------------

* The :command:`ctest_configure`, :command:`ctest_build`,
  :command:`ctest_test`, and :command:`ctest_memcheck` commands gained
  ``PRESET`` and ``PRESETS_FILE`` arguments to support using
  :manual:`presets <cmake-presets(7)>` for their :ref:`Dashboard Client` steps.

* The :variable:`CTEST_CONFIGURE_PRESET`, :variable:`CTEST_BUILD_PRESET`,
  :variable:`CTEST_TEST_PRESET`, :variable:`CTEST_PRESET`, and
  :variable:`CTEST_PRESETS_FILE` variables may be used to specify preset
  arguments to the above commands in a :ref:`Dashboard Client` script or
  on the :program:`ctest` command line via the
  :ref:`-D <ctest-option-D-var>` option.
