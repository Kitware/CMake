Ninja
-----

Generates a ``build.ninja`` file into the build tree.

Builtin Targets
^^^^^^^^^^^^^^^

``all``

  Depends on all targets required by the project, except those with
  :prop_tgt:`EXCLUDE_FROM_ALL` set to true.

``install``

  Runs the install step.

``install/strip``

  .. versionadded:: 3.7

    Runs the install followed by a ``CMAKE_STRIP`` command, if any.

    The ``CMAKE_STRIP`` variable will contain the platform's ``strip`` utility, which
    removes symbols information from generated binaries.

``install/parallel``

  .. versionadded:: 3.30

    Created only if the :prop_gbl:`INSTALL_PARALLEL` global property is ``ON``.
    Runs the install step for each subdirectory independently and in parallel.

``test_prep/<test-name>``

  .. versionadded:: 4.4

    Created only if the :variable:`CMAKE_TEST_BUILD_DEPENDS` variable is
    enabled. Builds all known build dependencies for the named test, including
    the executable target invoked by the test, targets referenced by generator
    expressions in the test command, and explicit ``BUILD_DEPENDS`` entries.

    Tests with names that are not valid target names are excluded. If multiple
    tests in different directories share the same name, their dependencies are
    merged into one ``test_prep/<test-name>`` target.

``test_prep/all``

  .. versionadded:: 4.4

    Created only if the :variable:`CMAKE_TEST_BUILD_DEPENDS` variable is
    enabled. Depends on every generated ``test_prep/<test-name>`` target.

For each subdirectory ``sub/dir`` of the project, additional targets
are generated:

``sub/dir/all``

  .. versionadded:: 3.6

    Depends on all targets required by the subdirectory.

``sub/dir/install``

  .. versionadded:: 3.7

    Runs the install step in the subdirectory, if any.

``sub/dir/install/strip``

  .. versionadded:: 3.7

    Runs the install step in the subdirectory followed by a ``CMAKE_STRIP`` command,
    if any.

``sub/dir/test``

  .. versionadded:: 3.7

    Runs the test step in the subdirectory, if any.

``sub/dir/package``

  .. versionadded:: 3.7

    Runs the package step in the subdirectory, if any.

Fortran Support
^^^^^^^^^^^^^^^

.. versionadded:: 3.7

The ``Ninja`` generator conditionally supports Fortran when the ``ninja``
tool is at least version 1.10 (which has the required features).

Swift Support
^^^^^^^^^^^^^

.. versionadded:: 3.15

The Swift support is experimental, not considered stable, and may change
in future releases of CMake.

See Also
^^^^^^^^

.. versionadded:: 3.17
  The :generator:`Ninja Multi-Config` generator is similar to the ``Ninja``
  generator, but generates multiple configurations at once.
