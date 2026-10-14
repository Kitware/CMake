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

``install_prep/<component>``

  .. versionadded:: 4.5

    Created if the project has install rules, unless the
    :variable:`CMAKE_SKIP_INSTALL_PREP_TARGETS` or
    :variable:`CMAKE_SKIP_INSTALL_RULES` variable is enabled. Builds all known
    build dependencies for the named install component. Components whose
    names cannot be represented safely are excluded. See
    :ref:`Install Preparation Targets` and :ref:`Target Names`.

``install_prep/all``

  .. versionadded:: 4.5

    Created if the project has install rules, unless the
    :variable:`CMAKE_SKIP_INSTALL_PREP_TARGETS` or
    :variable:`CMAKE_SKIP_INSTALL_RULES` variable is enabled. Builds all known
    build dependencies for an installation that does not select a component.
    See :ref:`Install Preparation Targets`.

``test_prep/<test-name>``

  .. versionadded:: 4.5

    Created unless the :variable:`CMAKE_SKIP_TEST_PREP_TARGETS` variable is
    enabled. Builds all known build dependencies for the named test, including
    the executable target invoked by the test, targets referenced by generator
    expressions in the test command, and explicit ``BUILD_DEPENDS`` entries.
    A ``BUILD_DEPENDS`` file is built by building the target whose build
    produces it.

    Tests with names that are not valid target names, and tests whose names
    contain a ``:`` character, are excluded. If multiple tests in different
    directories share the same name, their dependencies are merged into one
    ``test_prep/<test-name>`` target. See the :manual:`cmake-buildsystem(7)`
    manual for more details on :ref:`Target Names`.

    Individual ``test_prep/<test-name>`` targets are not created for tests
    discovered at test time, because their names are not known during
    generation.

``test_prep/all``

  .. versionadded:: 4.5

    Created unless the :variable:`CMAKE_SKIP_TEST_PREP_TARGETS` variable is
    enabled. Depends on every generated ``test_prep/<test-name>`` target. Also
    builds dependencies recorded by the :prop_dir:`CMAKE_TEST_BUILD_DEPENDS`
    directory property.
