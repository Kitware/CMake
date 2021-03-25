Ninja
-----

Generates ``build.ninja`` files.

A ``build.ninja`` file is generated into the build tree.  Use the ninja
program to build the project through the ``all`` target and install the
project through the ``install`` (or ``install/strip``) target.

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

    The ``CMAKE_STRIP`` variable will contain the platform's ``strip`` utility, which
    removes symbols information from generated binaries.

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
