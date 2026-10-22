CTEST_BUILD_FLAGS
-----------------

Specify the CTest ``BuildFlags`` setting
in a :manual:`ctest(1)` :ref:`Dashboard Client` script,
or on the :program:`ctest` command line via the :ctest-dashboard-option:`-D` option.

The :command:`ctest_build` command passes these flags to the native
build tool when :variable:`CTEST_BUILD_COMMAND` is not set.  The
``FLAGS`` option of the command takes priority over this variable.
