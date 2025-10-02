Step 9: Installation Commands and Concepts
==========================================

Projects need to do more than build and test their code, they need to make it
available to consumers. The layout of files in the build tree is unsuitable
for consumption by other projects, binaries are in unexpected places, header
files are located far away in the source tree, and there's no clear way
to discover what targets are provided or how to use them.

This translation, moving artifacts from the source and build trees into a final
layout suitable for consumption, is known as installation. CMake supports a
complete installation workflow as part of the project description, controlling
both the layout of artifacts in the install tree, and reconstructing targets
for other CMake projects which want to consume the libraries provided by the
install tree.

Background
^^^^^^^^^^

All CMake installation goes through a single command, :command:`install`, which
is split into many subcommands responsible for various aspects of the
installation process. For target-based CMake workflows, it is mostly sufficient
to rely on installing targets themselves with :command:`install(TARGETS)`
instead of resorting to manually moving files with :command:`install(FILES)`
or :command:`install(DIRECTORY)`.

.. note::
  This is why we need to add ``FILES`` to header sets which are intended to be
  installed. CMake needs to be able to locate the files when their associated
  target is installed.

CMake divides target-based installation into various artifact kinds. The
available artifact kinds (in CMake 3.23) are:

  ``ARCHIVE``
    Static libraries (``.a`` / ``.lib``), DLL import libraries (``.lib``), and
    a handful of other "archive-like" objects.

  ``LIBRARY``
    Shared libraries (``.so``), modules, and other dynamically loadable
    objects. **Not** Window's DLL files (``.dll``) or MacOS frameworks.

  ``RUNTIME``
    Executables of all kinds except MacOS bundles; and Window's DLLs (``.dll``).

  ``OBJECT``
    Objects from ``OBJECT`` libraries.

  ``FRAMEWORK``
    Both static and shared MacOS frameworks

  ``BUNDLE``
    MacOS bundle executables

  ``PUBLIC_HEADER`` / ``PRIVATE_HEADER`` / ``RESOURCE``
    Files described by the :prop_tgt:`PUBLIC_HEADER`, :prop_tgt:`PRIVATE_HEADER`
    and :prop_tgt:`RESOURCE` target properties, typically used with MacOS
    frameworks

  ``FILE_SET <set-name>``
    A file set associated with the target. This is how headers are typically
    installed.

Most important artifact kinds have known destinations which CMake will default
to unless instructed to do otherwise. For example, ``RUNTIME`` will be installed
to the location named by :module:`CMAKE_INSTALL_BINDIR <GNUInstallDirs>`, if
the variable is available, otherwise they default to ``bin``.

The full list of artifact kind default destinations is described in the
following table.

=============================== =============================== ======================
    Target Type                              Variable           Built-In Default
=============================== =============================== ======================
``RUNTIME``                     ``${CMAKE_INSTALL_BINDIR}``     ``bin``
``LIBRARY``                     ``${CMAKE_INSTALL_LIBDIR}``     ``lib``
``ARCHIVE``                     ``${CMAKE_INSTALL_LIBDIR}``     ``lib``
``PRIVATE_HEADER``              ``${CMAKE_INSTALL_INCLUDEDIR}`` ``include``
``PUBLIC_HEADER``               ``${CMAKE_INSTALL_INCLUDEDIR}`` ``include``
``FILE_SET`` (type ``HEADERS``) ``${CMAKE_INSTALL_INCLUDEDIR}`` ``include``
=============================== =============================== ======================

For the most part, projects should leave the defaults alone unless they need to
install to a specific subdirectory of a default location.

CMake does not define the ``CMAKE_INSTALL_<dir>`` variables by default. If a
project wishes to dictate installing to a subdirectory of one of these
locations, it is necessary to include the :module:`GNUInstallDirs` module, which
will provide values for all ``CMAKE_INSTALL_<dir>`` variables that have not
already been defined.

Exercise 1 - Installing Artifacts
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For modern, target-based CMake projects installation of artifacts is trivial
and consists of a single call to :command:`install(targets)`.

.. code-block:: cmake

  install(
    TARGETS MyApp MyLib

    FILE_SET HEADERS
    FILE_SET anotherHeaderFileSet
  )

Most artifact kinds are installed by default and do not need to be listed in
the :command:`install` command. However, ``FILE_SET``\ s must be named to let
CMake know you want to install. In the above example we install two file
sets, one named ``HEADERS`` and another named ``anotherHeaderFileSet``.

When named, an artifact kind can be given various options, such as a destination.

.. code-block:: cmake

  include(GNUInstallDirs)

  install(
    TARGETS MyApp MyLib

    RUNTIME
      DESTINATION ${CMAKE_INSTALL_BINDIR}/Subfolder

    FILE_SET HEADERS
  )

This will install the ``MyApp`` target to ``bin/Subfolder`` (if the packager
hasn't changed :module:`CMAKE_INSTALL_BINDIR <GNUInstallDirs>`).

Importantly, if the ``OBJECT`` artifact kind is never given a destination, it
will act like an ``INTERFACE`` library, only installing its headers.

Goal
----

Install the artifacts for the libraries and executables (except tests) described
in the tutorial project.

Helpful Resources
-----------------

* :command:`install`

Files to Edit
-------------

* ``CMakeLists.txt``

Getting Started
---------------

The ``Help/guide/tutorial/Step9`` directory contains the complete, recommended
solution to ``Step8``. Complete ``TODO 1`` and ``TODO 2``.

Build and Run
-------------

No special configuration is needed, configure and build as usual.

.. code-block:: console

  cmake --preset tutorial
  cmake --build build

We can verify the installation is correct with :option:`cmake --install`.

.. code-block:: console

  cmake --install build --prefix install

The ``install`` folder should be populated correctly for our artifacts.

Solution
--------

First we add an :command:`install(TARGETS)` for the conditionally built,
thus conditionally installed, ``Tutorial`` executable.

.. raw:: html

  <details><summary>TODO 1 Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 1: CMakeLists.txt
  :name: CMakeLists.txt-install-tutorial

  if(TUTORIAL_BUILD_UTILITIES)
    add_subdirectory(Tutorial)
    install(
      TARGETS Tutorial
    )
  endif()

.. raw:: html

  </details>

Then we can install the rest of the targets.

.. raw:: html

  <details><summary>TODO 2 Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 2: CMakeLists.txt
  :name: CMakeLists.txt-install-libs

  install(
    TARGETS MathFunctions OpAdd OpMul OpSub MathLogger SqrtTable
    FILE_SET HEADERS
  )

.. raw:: html

  </details>

.. note::
  We could add :command:`install(TARGETS)` commands locally to each subfolder
  where the targets are defined. This would be typical in very large projects
  where keeping track of all the installable targets is difficult.

It might seem unnecessary to install the ``SqrtTable`` and ``MathLogger``,
and it is at this stage. Due to how CMake models target relationships, when we
reconstruct the target model in the next exercise we will need these targets to
be available.

Exercise 2 - Exporting Targets
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This raw collection of installed files is a good start, but we lose the CMake
target model. These are effectively no better than the pre-compiled vendored
libraries we discussed in ``Step 4``. We need some way for other projects to
reconstruct our targets from what we have provided in the install tree.

The mechanism CMake provides to solve this is a CMakeLang file known as a
"target export file". It is created by the :command:`install(EXPORT)`
command.

.. code-block:: cmake

  install(
    TARGETS MyApp MyLib
    EXPORT MyProjectTargets
  )

  include(GNUInstallDirs)

  install(
    EXPORT MyProjectTargets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MyProject
    NAMESPACE MyProject::
  )

There are several parts to the above example. Firstly the
:command:`install(TARGETS)` command takes an export name, basically a list to
add the installed targets to.

Later, the :command:`install(EXPORT)` command consumes this list of targets
to generate the target export file. This will be a file named
``<ExportName>.cmake`` located in the provided ``DESTINATION``. The
``DESTINATION`` provided in this example is the conventional one, but any
location searched by the :command:`find_package` command is valid.

Finally, the targets created by the target export file will be prefixed with the
``NAMESPACE`` string, ie they will be of the form ``<NAMESPACE><TargetName>``.
It is conventional for this to be the project name followed by two colons.

For reasons that will become more obvious in future steps, we typically don't
consume this file directly. Instead we have a file named
``<ProjectName>Config.cmake`` consume it via :command:`include()`.

.. code-block:: cmake

  include(${CMAKE_CURRENT_LIST_DIR}/MyProjectTargets.cmake)

.. note::
  The :variable:`CMAKE_CURRENT_LIST_DIR` variable names the directory that the
  currently running CMake Language file is inside of, regardless of how that
  file was included or launched.

Then this file is installed alongside the target export with
:command:`install(FILES)`.

.. code-block:: cmake

  install(
    FILES
      cmake/MyProjectConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MyProject
  )

.. note::
  The name of this file and its location are dictated by the discovery
  semantics of the :command:`find_package` command, which we will discuss more
  in the next step.

Goal
----

Export the Tutorial project targets so other projects may consume them.

Helpful Resources
-----------------

* :command:`install`
* :module:`GNUInstallDirs`
* :variable:`CMAKE_CURRENT_LIST_DIR`

Files to Edit
-------------

* ``CMakeLists.txt``
* ``cmake/TutorialConfig.cmake``

Getting Started
---------------

Continue editing the files in the ``Help/guide/tutorial/Step9`` directory.
Complete ``TODO 3`` through ``TODO 8``.

Build and Run
-------------

The build command is sufficient to reconfigure the project.

.. code-block:: console

  cmake --build build

We can verify the installation is correct with :option:`cmake --install`.

.. note::

  As with CTest, when using multi-config generator, eg Visual Studio, it will be
  necessary to specify a configuration with
  ``cmake --install --config <config> <remaining flags>``, where
  ``<config>`` is a value like ``Debug`` or ``Release``. This is true whenever
  using a multi-config generator, and won't be called out specifically in
  future commands.

.. code-block:: console

  cmake --install build --prefix install

.. note::
  CMake won't update files which have not changed, only installing new or
  updated files from the build and source trees.

The ``install`` folder should be populated correctly for our artifacts and
export files. We'll demonstrate how to use these files in the next step.

Solution
--------

First we add the ``Tutorial`` target to the ``TutorialTargets`` export.

.. raw:: html

  <details><summary>TODO 3 Click to show/hide answer</summary>

.. literalinclude:: Step10/TutorialProject/CMakeLists.txt
  :caption: TODO 3: CMakeLists.txt
  :name: CMakeLists.txt-install-tutorial-export
  :language: cmake
  :start-at: install(
  :end-at: )

.. raw:: html

  </details>

Soon we will need access to the ``CMAKE_INSTALL_<dir>`` variables, so next
we include the :module:`GNUInstallDirs` module.

.. raw:: html

  <details><summary>TODO 4 Click to show/hide answer</summary>

.. literalinclude:: Step10/TutorialProject/CMakeLists.txt
  :caption: TODO 4: CMakeLists.txt
  :name: CMakeLists.txt-gnuinstalldirss
  :language: cmake
  :start-at: include(GNUInstallDirs)
  :end-at: include(GNUInstallDirs)

.. raw:: html

  </details>

Now we add the rest of our targets to the ``TutorialTargets`` export.

.. raw:: html

  <details><summary>TODO 5 Click to show/hide answer</summary>

.. literalinclude:: Step10/TutorialProject/CMakeLists.txt
  :caption: TODO 5: CMakeLists.txt
  :name: CMakeLists.txt-install-libs-export
  :language: cmake
  :start-at: TARGETS MathFunctions
  :end-at: )
  :prepend: install(

.. raw:: html

  </details>

Next we install the export itself, to generate our target export file.

.. raw:: html

  <details><summary>TODO 6 Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 6: CMakeLists.txt
  :name: CMakeLists.txt-install-export

  install(
    EXPORT TutorialTargets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Tutorial
    NAMESPACE Tutorial::
  )

.. raw:: html

  </details>

And then we install our "config" file, which we will use to include our target
export file.

.. raw:: html

  <details><summary>TODO 7 Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 7: CMakeLists.txt
  :name: CMakeLists.txt-install-config

  install(
    FILES
      cmake/TutorialConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Tutorial
  )

.. raw:: html

  </details>

Finally we can add the necessary :command:`include` command to the config file.

.. raw:: html

  <details><summary>TODO 8 Click to show/hide answer</summary>

.. literalinclude:: Step10/TutorialProject/cmake/TutorialConfig.cmake
  :caption: TODO 8: cmake/TutorialConfig.cmake
  :name: cmake/TutorialConfig.cmake
  :language: cmake
  :start-at: include
  :end-at: include

.. raw:: html

  </details>

Exercise 3 - Exporting a Version File
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When importing CMake targets from a target export file, there is no way to
"bail out" or "undo" the operation. If it turns out a package is a wrong or
incompatible version for the one we requested, we'll be stuck with any
side-effects incurred while we learned that version information.

The answer CMake provides for this problem is a light-weight version file which
only describes this version compatibility information, which can be checked
before CMake commits to fully importing the file.

CMake provides helper modules and scripts for generating these version files,
namely the :module:`CMakePackageConfigHelpers` module.

.. code-block:: cmake

  include(CMakePackageConfigHelpers)

  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/MyProjectConfigVersion.cmake
    COMPATIBILITY ExactVersion
  )

The available versions are:

* ``AnyNewerVersion``
* ``SameMajorVersion``
* ``SameMinorVersion``
* ``ExactVersion``

Additionally packages can mark themselves as ``ARCH_INDEPENDENT``, intended for
packages which ship no binaries which would tie them to a specific machine
architecture.

By default, the ``VERSION`` used by ``write_basic_package_version_file()`` is
the ``VERSION`` number given to the :command:`project` command.

Goal
----

Export a version file for the Tutorial project.

Helpful Resources
-----------------

* :command:`project`
* :command:`install`
* :module:`CMakePackageConfigHelpers`
* :variable:`PROJECT_VERSION`

Files to Edit
-------------

* ``CMakeLists.txt``

Getting Started
---------------

Continue editing the files in the ``Help/guide/tutorial/Step9`` directory.
Complete ``TODO 9`` through ``TODO 12``.

Build and Run
-------------

Rebuild and install as done previously.

.. code-block:: console

  cmake --build build
  cmake --install build --prefix install

The ``install`` folder should be populated correctly with our newly generated
and installed version file.

Solution
--------

First we add a ``VERSION`` parameter to the :command:`project` command.

.. raw:: html

  <details><summary>TODO 9 Click to show/hide answer</summary>

.. literalinclude:: Step10/TutorialProject/CMakeLists.txt
  :caption: TODO 9: CMakeLists.txt
  :name: CMakeLists.txt-project-version
  :language: cmake
  :start-at: project(
  :end-at: )

.. raw:: html

  </details>

Next we include the :module:`CMakePackageConfigHelpers` modules and use it
to generate the config version file.

.. raw:: html

  <details><summary>TODO 10-11 Click to show/hide answer</summary>

.. literalinclude:: Step10/TutorialProject/CMakeLists.txt
  :caption: TODO 10-11: CMakeLists.txt
  :name: CMakeLists.txt-write_basic_package_version_file
  :language: cmake
  :start-at: include(CMakePackageConfigHelpers
  :end-at: COMPATIBILITY ExactVersion
  :append: )

.. raw:: html

  </details>

Finally we add the config version file to the list of files to be installed.

.. raw:: html

  <details><summary>TODO 12 Click to show/hide answer</summary>

.. literalinclude:: Step10/TutorialProject/CMakeLists.txt
  :caption: TODO 12: CMakeLists.txt
  :name: CMakeLists.txt-install-version-config
  :language: cmake
  :start-at: FILES
  :end-at: )
  :prepend: install(

.. raw:: html

  </details>
