Step 5: In-Depth CMake Library Concepts
=======================================

While executables are mostly one-size-fits-all, libraries come in many
different forms. There are static archives, shared objects, modules,
object libraries, header-only libraries, and libraries which describe advanced
CMake properties to be inherited by other targets, just to name a few.

In this step you will learn about some of the most common kinds of libraries
that CMake can describe. This will cover most of the in-project uses of
:command:`add_library`. Libraries which are imported from dependencies (or
exported by the project to be consumed as a dependency) will be covered in
later steps.

Background
^^^^^^^^^^

As we learned in ``Step1``, the :command:`add_library` command accepts the name
of the library target to be created as its first argument. The second
argument is an optional ``<type>`` for which the following values are valid:

  ``STATIC``
    A :ref:`Static Library <Static Libraries>`:
    an archive of object files for use when linking other targets.

  ``SHARED``
    A :ref:`Shared Library <Shared Libraries>`:
    a dynamic library that may be linked by other targets and loaded
    at runtime.

  ``MODULE``
    A :ref:`Module Library <Module Libraries>`:
    a plugin that may not be linked by other targets, but may be
    dynamically loaded at runtime using dlopen-like functionality.

  ``OBJECT``
    An :ref:`Object Library <Object Libraries>`:
    a collection of object files which have not been archived or linked
    into a library.

  ``INTERFACE``
    An :ref:`Interface Library <Interface Libraries>`:
    a library target which specifies usage requirements for dependents but
    does not compile sources and does not produce a library artifact on disk.

In addition, there are ``IMPORTED`` libraries which describe library targets
from foreign projects or modules, imported into the current project. We will
cover these briefly in later steps.

``MODULE`` libraries are most commonly found in plugin systems, or as extensions
to runtime-loading languages like Python or Javascript. They act very similar to
normal shared libraries, except they cannot be directly linked by other targets.
They are sufficiently similar that we won't cover them in further depth here.

Exercise 1 - Static and Shared
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

While the :command:`add_library` command supports explicitly setting ``STATIC``
or ``SHARED``, and this is sometimes necessary, it is best to leave the second
argument empty for most "normal" libraries which can operate as either.

When not given a type, :command:`add_library` will create either a ``STATIC``
or ``SHARED`` library depending on the value of :variable:`BUILD_SHARED_LIBS`.
If :variable:`BUILD_SHARED_LIBS` is true, a ``SHARED`` library will be created,
otherwise it will be ``STATIC``.

.. code-block:: cmake

  add_library(MyLib-static STATIC)
  add_library(MyLib-shared SHARED)

  # Depends on BUILD_SHARED_LIBS
  add_library(MyLib)

This is desirable behavior, as it allows packagers to determine what kind of
library will be produced, and ensure dependents link to that version of the
library without needing to modify their source code. In some contexts, fully
static builds are appropriate, and in others shared libraries are desirable.

.. note::
  CMake does not define the :variable:`BUILD_SHARED_LIBS` variable by default,
  meaning without project or user intervention :command:`add_library` will
  produce ``STATIC`` libraries.

By leaving the second argument to :command:`add_library()` blank, projects
provide additional flexibility to their packagers and downstream dependents.

Goal
----

Build ``MathFunctions`` as a shared library.

.. note::
  On Windows, you might see warnings about an empty DLL, as ``MathFunctions``
  doesn't export any symbols.

Helpful Resources
-----------------

* :variable:`BUILD_SHARED_LIBS`

Files to Edit
-------------

There are no files to edit.

Getting Started
---------------

The ``Help/guide/tutorial/Step5`` directory contains the complete, recommended
solution to ``Step4``. This step is about building the ``MathFunctions``
library, there are no ``TODOs`` necessary. You can proceed directly to the
build step.

Build and Run
-------------

We can configure using our preset, turning on :variable:`BUILD_SHARED_LIBS` with
a :option:`-D <cmake -D>` flag.

.. code-block:: console

  cmake --preset tutorial -DBUILD_SHARED_LIBS=ON

Then we can build only the ``MathFunctions`` library with
:option:`-t <cmake--build -t>`.

.. code-block:: console

  cmake --build build -t MathFunctions

Verify a shared library is produced for ``MathFunctions`` then reset
:variable:`BUILD_SHARED_LIBS`, either by reconfiguring with
``-DBUILD_SHARED_LIBS=OFF`` or deleting the ``CMakeCache.txt``.

Solution
--------

There are no changes to the project for this exercise.

Exercise 2 - Interface Libraries
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Interface libraries are those which only communicate usage requirements for
other targets, they do not build or produce any artifacts of their own. As such
all the properties of an interface library must themselves be interface
properties, specified with the ``INTERFACE`` :ref:`scope keywords <Target Command Scope>`.

.. code-block:: cmake

  add_library(MyInterface INTERFACE)
  target_compile_definitions(MyInterface INTERFACE MYINTERFACE_COMPILE_DEF)

The most common kind of interface library in C++ development is a header-only
library. Such libraries do not build anything, only providing the flags
necessary to discover their headers.

Goal
----

Add a header-only library to the tutorial project, and use it inside the
``Tutorial`` executable.

Helpful Resources
-----------------

* :command:`add_library`
* :command:`target_sources`

Files to Edit
-------------

* ``MathFunctions/MathLogger/CMakeLists.txt``
* ``MathFunctions/CMakeLists.txt``
* ``MathFunctions/MathFunctions.cxx``

Getting Started
---------------

In our previous discussions of :command:`target_sources(FILE_SET)`, we noted
we can omit the ``TYPE`` parameter if the file set's name is the same as the
file set's type. We also said we can omit the ``BASE_DIRS`` parameter if
we want to use the current source directory as the only base directory.

We're ready to introduce a third shortcut, we only need to include the ``FILES``
parameter if the headers are intended to be installed, such as public headers
of a library.

The ``MathLogger`` headers in this exercise are only used internally by the
``MathFunctions`` implementation. They will not be installed. This should
make for a very abbreviated call to :command:`target_sources(FILE_SET)`.

.. note::
  The headers will be discovered by the compiler's dependency scanner to ensure
  correct incremental builds. It can be useful to list header files in these
  contexts anyway, as the list can be used to generate metadata some IDEs
  rely on.

You can begin editing the ``Step5`` directory. Complete ``TODO 1`` through
``TODO 7``.

Build and Run
-------------

The preset has already been updated to use ``mathfunctions::sqrt`` instead of
``std::sqrt``. We can build and configure as usual.

.. code-block:: console

  cmake --preset tutorial
  cmake --build build

Verify that the ``Tutorial`` output now uses the logging framework.

Solution
--------

First we add a new ``INTERFACE`` library named ``MathLogger``.

.. raw:: html

  <details><summary>TODO 1: Click to show/hide answer</summary>

.. literalinclude:: Step6/MathFunctions/MathLogger/CMakeLists.txt
  :caption: TODO 1: MathFunctions/MathLogger/CMakeLists.txt
  :name: MathFunctions/MathLogger/CMakeLists.txt-add_library
  :language: cmake
  :start-at: add_library
  :end-at: add_library

.. raw:: html

  </details>

Then we add the appropriate :command:`target_sources` call to capture the
header information. We give this file set the name ``HEADERS`` so we can
omit the ``TYPE``, we don't need ``BASE_DIRS`` as we will use the default
of the current source directory, and we can exclude the ``FILES`` list because
we don't intend to install the library.

.. raw:: html

  <details><summary>TODO 2: Click to show/hide answer</summary>

.. literalinclude:: Step6/MathFunctions/MathLogger/CMakeLists.txt
  :caption: TODO 2: MathFunctions/MathLogger/CMakeLists.txt
  :name: MathFunctions/MathLogger/CMakeLists.txt-target_sources
  :language: cmake
  :start-at: target_sources(
  :end-at: )

.. raw:: html

  </details>

Now we can add the ``MathLogger`` library to the ``MathFunctions`` linked
libraries, and at the ``MathLogger`` folder to the project.

.. raw:: html

  <details><summary>TODO 3-4: Click to show/hide answer</summary>

.. literalinclude:: Step6/MathFunctions/CMakeLists.txt
  :caption: TODO 3: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-link-mathlogger
  :language: cmake
  :start-at: target_link_libraries(
  :end-at: MathLogger
  :append: )

.. literalinclude:: Step6/MathFunctions/CMakeLists.txt
  :caption: TODO 4: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-add-mathlogger
  :language: cmake
  :start-at: add_subdirectory(MathLogger
  :end-at: add_subdirectory(MathLogger

.. raw:: html

  </details>

Finally we can update ``MathFunctions.cxx`` to take advantage of the new logger.

.. raw:: html

  <details><summary>TODO 5-7: Click to show/hide answer</summary>

.. literalinclude:: Step6/MathFunctions/MathFunctions.cxx
  :caption: TODO 5: MathFunctions/MathFunctions.cxx
  :name: MathFunctions/MathFunctions.cxx-mathlogger-header
  :language: c++
  :start-at: cmath
  :end-at: MathLogger

.. literalinclude:: Step6/MathFunctions/MathFunctions.cxx
  :caption: TODO 6: MathFunctions/MathFunctions.cxx
  :name: MathFunctions/MathFunctions.cxx-mathlogger-logger
  :language: c++
  :start-at: mathlogger::Logger Logger
  :end-at: mathlogger::Logger Logger

.. literalinclude:: Step6/MathFunctions/MathFunctions.cxx
  :caption: TODO 7: MathFunctions/MathFunctions.cxx
  :name: MathFunctions/MathFunctions.cxx-mathlogger-code
  :language: c++
  :start-at: Logger.Log(std::format("Computing sqrt of {} to be {}\n"
  :end-at: std::format
  :dedent: 4

.. raw:: html

  </details>

Exercise 3 - Object Libraries
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Object libraries have several advanced uses, but also tricky nuances which
are difficult to fully enumerate in the scope of this tutorial.

.. code-block:: cmake

  add_library(MyObjects OBJECT)

The most obvious drawback to object libraries is the objects themselves cannot
be transitively linked. If an object library appears in the
:prop_tgt:`INTERFACE_LINK_LIBRARIES` of a target, the dependents which link that
target will not "see" the objects. The object library will act like an
``INTERFACE`` library in such contexts. In the general case, object libraries
are only suitable for ``PRIVATE`` or ``PUBLIC`` consumption via
:command:`target_link_libraries`.

A common use case for object libraries is coalescing several library targets
into a single archive or shared library object. Even within a single project
libraries may be maintained as different targets for a variety of reasons, such
as belonging to different teams within an organization. However, it may be
desirable to distribute these as a single consumer-facing binary. Object
libraries make this possible.

Goal
----

Add several object libraries to the ``MathFunctions`` library.

Helpful Resources
-----------------

* :command:`target_link_libraries`
* :command:`add_subdirectory`

Files to Edit
-------------

* ``MathFunctions/CMakeLists.txt``
* ``MathFunctions/MathFunctions.h``
* ``Tutorial/Tutorial.cxx``

Getting Started
---------------

Several extensions for our ``MathFunctions`` library have been made available
(we can imagine these coming from other teams in our organization). Take
a minute to look at the targets made available in ``MathFunctions/MathExtensions``.
Then complete ``TODO 8`` through ``TODO 11``.

Build and Run
-------------

There's no reconfiguration needed, we can build as usual.

.. code-block:: console

  cmake --build build

Verify the output of ``Tutorial`` now includes the verification message. Also
take a minute to inspect the build directory under
``build/MathFunctions/MathExtensions``. You should find that, unlike
``MathFunctions``, no archives are produced for any of the object libraries.

Solution
--------

First we will add links for all the object libraries to ``MathFunctions``.
These are ``PUBLIC``, because we want the objects to be added to the
``MathFunctions`` library as part of its own build step, and we want the
headers to be available to consumers of the library.

Then we add the ``MathExtensions`` subdirectoy to the project.

.. raw:: html

  <details><summary>TODO 8-9: Click to show/hide answer</summary>

.. literalinclude:: Step6/MathFunctions/CMakeLists.txt
  :caption: TODO 8: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-link-objects
  :language: cmake
  :start-at: target_link_libraries(
  :end-at: )

.. literalinclude:: Step6/MathFunctions/CMakeLists.txt
  :caption: TODO 9: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-add-objs
  :language: cmake
  :start-at: add_subdirectory(MathExtensions
  :end-at: add_subdirectory(MathExtensions

.. raw:: html

  </details>


To make the extensions available to consumers, we include their headers in the
``MathFunctions.h`` header.

.. raw:: html

  <details><summary>TODO 10: Click to show/hide answer</summary>

.. literalinclude:: Step6/MathFunctions/MathFunctions.h
  :caption: TODO 10: MathFunctions/MathFunctions.h
  :name: MathFunctions/MathFunctions.h-include-objects
  :language: c++
  :start-at: OpAdd
  :end-at: OpSub

.. raw:: html

  </details>

Finally we can take advantage of the extensions in the ``Tutorial`` program.

.. raw:: html

  <details><summary>TODO 11: Click to show/hide answer</summary>

.. literalinclude:: Step6/Tutorial/Tutorial.cxx
  :caption: TODO 11: Tutorial/Tutorial.cxx
  :name: Tutorial/Tutorial.cxx-use-objects
  :language: c++
  :start-at: OpMul
  :end-at: checkValue);
  :dedent: 2

.. raw:: html

  </details>
