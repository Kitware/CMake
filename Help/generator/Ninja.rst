Ninja
-----

Generates ``build.ninja`` files.

A ``build.ninja`` file is generated into the build tree.  Use the ninja
program to build the project through the ``all`` target and install the
project through the ``install`` (or ``install/strip``) target.

For each subdirectory ``sub/dir`` of the project, additional targets
are generated:

``sub/dir/all``
  Depends on all targets required by the subdirectory.

``sub/dir/install``
  Runs the install step in the subdirectory, if any.

``sub/dir/install/strip``
  Runs the install step in the subdirectory followed by a ``CMAKE_STRIP`` command,
  if any.

  The ``CMAKE_STRIP`` variable will contain the platform's ``strip`` utility, which
  removes symbols information from generated binaries.

``sub/dir/test``
  Runs the test step in the subdirectory, if any.

``sub/dir/package``
  Runs the package step in the subdirectory, if any.

Fortran Support
^^^^^^^^^^^^^^^

The ``Ninja`` generator conditionally supports Fortran when the ``ninja``
tool has the required features.  As of this version of CMake the needed
features have not been integrated into upstream Ninja.  Kitware maintains
a branch of Ninja with the required features on `github.com/Kitware/ninja`_.

.. _`github.com/Kitware/ninja`: https://github.com/Kitware/ninja/tree/features-for-fortran#readme

See Also
^^^^^^^^

The :generator:`Ninja Multi-Config` generator is similar to the ``Ninja``
generator, but generates multiple configurations at once.
