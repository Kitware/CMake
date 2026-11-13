CTEST_CONFIGURE_OPTIONS
-----------------------

.. versionadded:: 4.6

Specify a :ref:`semicolon-separated list <CMake Language Lists>` of
command-line options for the :command:`ctest_configure` command to pass to
:manual:`cmake(1)` in a :manual:`ctest(1)` :ref:`Dashboard Client` script,
or on the :program:`ctest` command line via the :ctest-dashboard-option:`-D`
option.

If the :command:`ctest_configure` command is given an explicit ``OPTIONS``
argument, that argument takes precedence and this variable is ignored.

This variable is ignored when :variable:`CTEST_CONFIGURE_COMMAND` is used.

Example --- pass an extra cache entry in a single ``ctest`` invocation::

  ctest -T Configure -D CTEST_CONFIGURE_OPTIONS=-DFOO=ON
