Step 11: Miscellaneous Features
===============================

Some features don't fit well or aren't important enough to receive attention
in the main tutorial, but deserve mention. These exercises collect some of those
features. They should be considered "bonuses".

There are many CMake features that are not covered by the tutorial, some of
which are considered essential to the projects which use them. Others are
in common use by packagers but see little discussion among software developers
producing local builds.

This list is not an exhaustive discussion of what remains of CMake's
capabilities. It may grow or shrink with time and relevance.

Exercise 1: Target Aliases
^^^^^^^^^^^^^^^^^^^^^^^^^^

This tutorial focuses on installing dependencies and consuming them from an
install tree. It also recommends the use of package managers to facilitate
this process. However, for a variety of reasons both historical and
contemporary this is not always how CMake projects are consumed.

It is possible to vendor a dependency's source code entirely in a parent project
and consume it with :command:`add_subdirectory`. When performed, the target
names exposed are those used within the project, not those exported via
:command:`install(EXPORT)`. The target names will not have the namespace string
that command prefixes to targets.

Some projects wish to support this workflow with an interface consistent with
the one presented to :command:`find_package` consumers. CMake supports this via
:command:`add_library(ALIAS)` and :command:`add_executable(ALIAS)`.

.. code-block:: cmake

  add_library(MyLib INTERFACE)
  add_library(MyProject::MyLib ALIAS MyLib)

Goal
----

Add a library alias for the ``MathFunctions`` library.

Helpful Resources
-----------------

* :command:`add_library`

Files to Edit
-------------

* ``TutorialProject/MathFunctions/CMakeLists.txt``

Getting Started
---------------

For this step we will only be editing the ``TutorialProject`` project in the
``Step11`` folder. Complete ``TODO 1``.

Build and Run
-------------

To build the project we first need configure and install ``SimpleTest``.
Navigate to ``Help/guide/Step11/SimpleTest`` and run the appropriate commands.

.. code-block:: console

  cmake --preset tutorial
  cmake --install build

Then navigate to ``Help/guide/Step11/TutorialProject`` and perform the usual build.

.. code-block:: console

  cmake --preset tutorial
  cmake --build build

There should be no observable change in behavior from adding the alias.

Solution
--------

We add a single line to the ``MathFunctions`` CML.

.. raw:: html

  <details><summary>TODO 1 Click to show/hide answer</summary>

.. literalinclude:: Complete/TutorialProject/MathFunctions/CMakeLists.txt
  :caption: TODO 1: TutorialProject/MathFunctions/CMakeLists.txt
  :name: TutorialProject/MathFunctions/CMakeLists.txt-alias
  :language: cmake
  :start-at: ALIAS
  :end-at: ALIAS

.. raw:: html

  </details>

Exercise 2: Generator Expressions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:manual:`Generator expressions <cmake-generator-expressions(7)>` are a
complicated domain-specific language supported in some contexts within CMake.
They are most easily understood as deferred-evaluation conditionals, they
express requirements where the inputs to determine the correct behavior are not
known during the CMake configuration stage.

.. note::
  This is where generator expressions get their name, they are evaluated when
  the underlying build system is being generated.

Generator expressions were commonly used in combination with
:command:`target_include_directories` to express include directory requirements
across the build and install tree, but file sets have superseded this use case.
Their most common applications now are in multi-config generators and
intricate dependency injection systems.

.. code-block:: cmake

  target_compile_definitions(MyApp PRIVATE "MYAPP_BUILD_CONFIG=$<CONFIG>")

Goal
----

Add a generator expression to ``SimpleTest`` that checks the build configuration
inside a compile definition.

Helpful Resources
-----------------

* :command:`target_compile_definitions`
* :manual:`cmake-generator-expressions(7)`

Files to Edit
-------------

* ``SimpleTest/CMakeLists.txt``

Getting Started
---------------

For this step we will only be editing the ``SimpleTest`` project in the
``Step11`` folder. Complete ``TODO 2``.

Build and Run
-------------

To build the project we first need configure and install ``SimpleTest``.
Navigate to ``Help/guide/Step11/SimpleTest`` and run the appropriate commands.

.. code-block:: console

  cmake --preset tutorial
  cmake --install build

Then navigate to ``Help/guide/Step11/TutorialProject`` and perform the usual build.

.. code-block:: console

  cmake --preset tutorial
  cmake --build build

When running the ``TestMathFunctions`` binary directly, we should a message
naming the build configuration used to build the executable (not necessarily the
same as configuration used to configure ``SimpleTest``). On single configuration
generators, the build configuration can be changed by setting
:variable:`CMAKE_BUILD_TYPE`.

Solution
--------

We add a single line to the ``SimpleTest`` CML.

.. raw:: html

  <details><summary>TODO 2 Click to show/hide answer</summary>

.. literalinclude:: Complete/SimpleTest/CMakeLists.txt
  :caption: TODO 2: SimpleTest/CMakeLists.txt
  :name: SimpleTest/CMakeLists.txt-target_compile_definitions
  :language: cmake
  :start-at: target_compile_definitions
  :end-at: target_compile_definitions

.. raw:: html

  </details>
