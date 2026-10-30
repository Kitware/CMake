CTEST_BUILD_TARGET
------------------

Specify the CTest ``BuildTarget`` setting
in a :manual:`ctest(1)` :ref:`Dashboard Client` script,
or on the :program:`ctest` command line via the :ctest-dashboard-option:`-D` option.

The :command:`ctest_build` command builds this target instead of the
default target when :variable:`CTEST_BUILD_COMMAND` is not set.  The
``TARGET`` option of the command takes priority over this variable.
