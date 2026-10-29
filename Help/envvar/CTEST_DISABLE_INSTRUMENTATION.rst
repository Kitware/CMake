CTEST_DISABLE_INSTRUMENTATION
-----------------------------

.. versionadded:: 4.6

.. include:: include/ENV_VAR.rst

When set to a true value, such as ``1``, ``True``, or ``ON``, this
environment variable disables all :manual:`CMake instrumentation
<cmake-instrumentation(7)>` data collection, indexing hooks, and callbacks.
This includes manual indexing with :option:`ctest --collect-instrumentation`.

This setting takes precedence over project-level and user-level queries,
the :command:`cmake_instrumentation` command, and the
:envvar:`CTEST_USE_INSTRUMENTATION` and
:envvar:`CTEST_USE_VERBOSE_INSTRUMENTATION` environment variables.
It also applies when building or testing an already-configured build tree.
Queries are still loaded and generated normally, but no instrumentation data
is collected or indexed.

Unset the variable, or set it to a false value such as ``0``, ``OFF``, or an
empty string, to allow instrumentation to operate normally.
