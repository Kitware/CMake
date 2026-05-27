CMAKE_TEST_BUILD_DEPENDS
------------------------

.. versionadded:: 4.4

Enable ``test_prep/<name>`` build targets for tests added by
:command:`add_test` when using the :ref:`Ninja Generators`.

When this variable is set to a true value, CMake generates a
``test_prep/<name>`` target for each eligible test and a ``test_prep/all``
target for all such tests. Building these targets ensures the test executable,
targets referenced by test command generator expressions, and explicit
``BUILD_DEPENDS`` entries are up-to-date before the test runs.

Tests whose names are not valid target names are excluded from this behavior.
If multiple tests in different directories share the same name, their
dependencies are merged into a single ``test_prep/<name>`` target.
