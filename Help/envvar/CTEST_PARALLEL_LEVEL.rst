CTEST_PARALLEL_LEVEL
--------------------

.. include:: include/ENV_VAR.rst

Specify the number of tests for CTest to run in parallel.
For example, if ``CTEST_PARALLEL_LEVEL`` is set to 8, CTest will run
up to 8 tests concurrently as if :manual:`ctest(1)` were invoked with the
:option:`--parallel 8 <ctest --parallel>` option.

.. versionchanged:: 3.29

  The value may be empty, or ``0``, to let CTest use a default level of
  parallelism, or unbounded parallelism, respectively, as documented by
  the :option:`ctest --parallel` option.

  CTest will interpret a whitespace-only string as empty.

  In CMake 3.28 and earlier, an empty or ``0`` value was equivalent to ``1``.

This environment variable is ignored if :option:`ctest --parallel` is given on
the command line, and is overridden by the :preset:`testPresets.execution.jobs`
field of a test preset, if that field is set.

A test preset may also set this variable itself, using its own
:preset:`testPresets.environment` field. Doing so is equivalent to setting
the variable in the calling process's environment, except that it takes
effect only for the preset's Test step, and only if ``execution.jobs`` is
not also set (which would take precedence, as noted above).

When a test preset is used by a :command:`ctest_test` command in a
:ref:`CTest Script`, that command's own ``PARALLEL_LEVEL`` argument, if given,
takes precedence over the preset's ``execution.jobs`` field. Together,
``PARALLEL_LEVEL`` and ``execution.jobs`` take precedence not only over this
environment variable, but over an explicit :option:`ctest --parallel` on the
command line as well. This differs from a preset used directly via
:option:`ctest --preset`, where an explicit ``--parallel`` always wins over the
preset's ``execution.jobs`` field.

See :manual:`ctest(1)` for more information on parallel test execution.
