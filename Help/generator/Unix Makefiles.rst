Unix Makefiles
--------------

Generates standard UNIX makefiles.

A hierarchy of UNIX makefiles is generated into the build tree.  Use
any standard UNIX-style make program to build the project through
the ``all`` target and install the project through the ``install``
(or ``install/strip``) target.

For each subdirectory ``sub/dir`` of the project a UNIX makefile will
be created, containing the following targets:

``all``
  Depends on all targets required by the subdirectory.

``install``
  Runs the install step in the subdirectory, if any.

``install/strip``
  Runs the install step in the subdirectory followed by a ``CMAKE_STRIP`` command,
  if any.

  The ``CMAKE_STRIP`` variable will contain the platform's ``strip`` utility, which
  removes symbols information from generated binaries.

``test``
  Runs the test step in the subdirectory, if any.

``package``
  Runs the package step in the subdirectory, if any.

.. versionadded:: 4.5

  When the :variable:`CMAKE_TEST_BUILD_DEPENDS` variable is enabled, the
  top-level makefile additionally provides the following targets:

  ``test_prep/<test-name>``
    Builds all known build dependencies for the test named ``<test-name>``
    added by :command:`add_test`, including the executable target invoked by
    the test, targets referenced by generator expressions in the test command,
    and explicit ``BUILD_DEPENDS`` entries.  A ``BUILD_DEPENDS`` file is built
    by building the target whose build produces it.

    Tests whose names are not valid target names, and tests whose names
    contain a ``:`` character, are excluded.  If multiple tests in different
    directories share the same name, their dependencies are merged into one
    ``test_prep/<test-name>`` target.

  ``test_prep/all``
    Depends on every generated ``test_prep/<test-name>`` target.
