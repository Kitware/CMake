Step 1: Getting Started with CMake
==================================

This first step in the CMake tutorial is intended as a quick-start into writing
useful builds for small projects with CMake. By the end, you will be able to
describe executables, libraries, source and header files, and the linkage
relationships between them using CMake.

Each exercise in this step will start with a discussion of the concepts and
commands needed for the exercise. Then, a goal and list of helpful resources are
provided. Each file in the ``Files to Edit`` section is in the ``Step1``
directory and contains one or more ``TODO`` comments. Each ``TODO`` represents
a line or two of code to change or add. The ``TODOs`` are intended to be
completed in numerical order, first complete  ``TODO 1`` then ``TODO 2``, etc.

.. note::
  Each step in the tutorial builds on the previous, but the steps are not
  strictly contiguous. Code not relevant to learning CMake, such as C++
  function implementations or CMake code outside the scope of the tutorial,
  will sometimes be added between steps.

The ``Getting Started`` section will give some helpful hints and guide you
through the exercise. Then the ``Build and Run`` section will walk step-by-step
through how to build and test the exercise. Finally, at the end of each exercise
the intended solution is reviewed.

Background
^^^^^^^^^^

Typical usage of CMake revolves around one or more files named
``CMakeLists.txt``. This file is sometimes referred to as a "lists file" or
"CML". Within a given software project, a ``CMakeLists.txt`` will exist within
any directory where we want to provide instructions to CMake on how to handle
files and operations local to that directory or subdirectories. Each consists of
a set of commands which describe some information or actions relevant to
building the software project.

Not every directory in a software project needs a CML, but it's strongly
recommended that the project root contains one. This will serve as the entry
point for CMake for its initial setup during configuration. This *root* CML
should always contain the same two commands at or near the top the file.

.. code-block:: cmake

  cmake_minimum_required(VERSION 3.23)

  project(MyProjectName)

The :command:`cmake_minimum_required` is a compatibility guarantee provided by
CMake to the project developer. When called, it ensures that CMake will adopt
the behavior of the listed version. If a later version of CMake is invoked on a
CML containing the above code, it will act exactly as if it were CMake 3.23.

The :command:`project` command is a conceptually simple command which provides a
complex function. It informs CMake that what follows is the description of a
distinct software project of a given name (as opposed to a shell-like script).
When CMake sees the :command:`project` command it performs various checks to
ensure the environment is suitable for building software; such as checking for
compilers and other build tooling, and discovering properties like the
endianness of the host and target machines.

.. note::
  While links to complete documentation are provided for every command, it is
  not intended the reader understand the full semantics of each CMake command
  they use. Effectively learning CMake, like any piece of software, is an
  incremental process.

The rest of this tutorial step will be chiefly concerned with the usage of four
more commands. The :command:`add_executable` and :command:`add_library` commands
for describing output artifacts the software project wants to produce, the
:command:`target_sources` command for associating input files with their
respective output artifacts, and the :command:`target_link_libraries` command
for associating output artifacts with one another.

These four commands are the backbone of most CMake usage. As we'll learn, they
are sufficient for describing the majority of a typical project's requirements.

Exercise 1 - Building an Executable
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The most basic CMake project is an executable built from a single source code
file. For simple projects like this, a ``CMakeLists.txt`` file with only
four commands is needed.

.. note::
  Although upper, lower and mixed case commands are supported by CMake,
  lower case commands are preferred and will be used throughout the tutorial.

The first two commands we have already introduced, :command:`cmake_minimum_required`
and :command:`project`. There is no usage of CMake where the first command in a
root CML will be anything other than :command:`cmake_minimum_required`. There
are some advanced usages where :command:`project` might not be the second
command in a CML, but for our purposes it always will be.

The next command we need is :command:`add_executable`.
This command creates a *target*. In CMake lingo, a target is a name the
developer gives to a collection of properties.

Some examples of properties a target might want to keep track of are:
  - The artifact kind (executable, library, header collection, etc)
  - Source files
  - Include directories
  - Output name of an executable or library
  - Dependencies
  - Compiler and linker flags

The mechanisms of CMake are often best understood as describing and manipulating
targets and their properties. There are many more properties than those listed
here. Documentation of CMake commands will often discuss their function in terms
of the target properties they operate on.

Targets themselves are simply names, a handle to this collection of properties.
Using the :command:`add_executable` command is as easy as specifying the name
we want to use for the target.

.. code-block:: cmake

  add_executable(MyProgram)

Now that we have a name for our target, we can start associating properties
with it like source files we want to build and link. The primary command for
this is :command:`target_sources`, which takes as arguments a target name
followed by one or more collections of files.

.. code-block:: cmake

  target_sources(MyProgram
    PRIVATE
      main.cxx
  )

.. note::
  Paths in CMake are generally either absolute, or relative to the
  :variable:`CMAKE_CURRENT_SOURCE_DIR`. We haven't talked about variables like
  that yet, so you can read this as "relative to the location of the current
  CML".

Each collection of files is prefixed by a :ref:`scope keyword <Target Command Scope>`.
We'll discuss the complete semantics of these keywords when we talk about
linking targets together, but the quick explanation is these describe how a
property should be inherited by dependents of our target.

Typically, nothing depends on an executable. Other programs and libraries don't
need to link to an executable, or inherit headers, or anything of that nature.
So the appropriate scope to use here is ``PRIVATE``, which informs CMake that
this property only belongs to ``MyProgram`` and is not inheritable.

.. note::
  This rule is true almost everywhere. Outside advanced and esoteric usages,
  the scope keyword for executables should *always* be ``PRIVATE``. The same
  holds for implementation files generally, regardless of whether the target
  is an executable or a library. The only target which needs to "see" the
  ``.cxx`` files is the target building them.

Goal
----

Understand how to create a simple CMake project with a single executable.

Helpful Resources
-----------------

* :command:`project`
* :command:`cmake_minimum_required`
* :command:`add_executable`
* :command:`target_sources`

Files to Edit
-------------

* ``CMakeLists.txt``

Getting Started
----------------

The source code for ``Tutorial.cxx`` is provided in the
``Help/guide/tutorial/Step1/Tutorial`` directory and can be used to compute the
square root of a number. This file does not need to be edited in this exercise.

In the parent directory, ``Help/guide/tutorial/Step1``, is a ``CMakeLists.txt``
file which you will complete. Start with ``TODO 1`` and work through ``TODO 4``.

Build and Run
-------------

Once ``TODO 1`` through ``TODO 4`` have been completed, we are ready to build
and run our project! First, run the :manual:`cmake <cmake(1)>` executable or the
:manual:`cmake-gui <cmake-gui(1)>` to configure the project and then build it
with your chosen build tool.

For example, from the command line we could navigate to the
``Help/guide/tutorial/Step1`` directory and invoke CMake for configuration
as follows:

.. code-block:: console

  cmake -B build

The :option:`-B <cmake -B>` flag tells CMake to use the given relative
path as the location to generate files and store artifacts during the build
process. If it is omitted, the current working directory is used. It is
generally considered bad practice to do "in-source" builds, placing these
generated files in the source tree itself.

Next, tell CMake to build the project with
:option:`cmake --build <cmake --build>`, passing it the same relative path
we did with the :option:`-B <cmake -B>` flag.

.. code-block:: console

  cmake --build build

The ``Tutorial`` executable will be built into the ``build`` directory. For
multi-config generators (e.g. Visual Studio), it might be placed in a
subdirectory such as ``build/Debug``.

Finally, try to use the newly built ``Tutorial``:

.. code-block:: console

  Tutorial 4294967296
  Tutorial 10
  Tutorial

.. note::
  Depending on the shell, the correct syntax may be ``Tutorial``,
  ``./Tutorial``, ``.\Tutorial``, or even ``.\Tutorial.exe``. For simplicity,
  the exercises will use ``Tutorial`` throughout.

Solution
--------

As mentioned above, a four command ``CMakeLists.txt`` is all that we need to get
up and running. The first line should be :command:`cmake_minimum_required`, to
set the CMake version as follows:

.. raw:: html

  <details><summary>TODO 1: Click to show/hide answer</summary>

.. literalinclude:: Step3/CMakeLists.txt
  :caption: TODO 1: CMakeLists.txt
  :name: CMakeLists.txt-cmake_minimum_required
  :language: cmake
  :start-at: cmake_minimum_required
  :end-at: cmake_minimum_required

.. raw:: html

  </details>

The next step to make a basic project is to use the :command:`project`
command as follows to set the project name and inform CMake we intend to build
software with this ``CMakeLists.txt``.

.. raw:: html

  <details><summary>TODO 2: Click to show/hide answer</summary>

.. literalinclude:: Step3/CMakeLists.txt
  :caption: TODO 2: CMakeLists.txt
  :name: CMakeLists.txt-project
  :language: cmake
  :start-at: project
  :end-at: project

.. raw:: html

  </details>

Now we can setup our executable target for the Tutorial with :command:`add_executable`.

.. raw:: html

  <details><summary>TODO 3: Click to show/hide answer</summary>

.. literalinclude:: Step3/Tutorial/CMakeLists.txt
  :caption: TODO 3: CMakeLists.txt
  :name: CMakeLists.txt-add_executable
  :language: cmake
  :start-at: add_executable
  :end-at: add_executable

.. raw:: html

  </details>

Finally, we can associate our source file with the Tutorial executable target
using :command:`target_sources`.

.. raw:: html

  <details><summary>TODO 4: Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 4: CMakeLists.txt
  :name: CMakeLists.txt-target_sources

  target_sources(Tutorial
    PRIVATE
      Tutorial/Tutorial.cxx
  )


.. raw:: html

  </details>

Exercise 2 - Building a Library
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

We only need to introduce one more command to build a library,
:command:`add_library`. This works exactly like :command:`add_executable`, but
for libraries.

.. code-block:: cmake

  add_library(MyLibrary)

However, now is a good time to introduce header files. Header files are not
directly built as translation units, which is to say they are not a *build*
requirement. They are a *usage* requirement. We need to know about header files
in order to build other parts of a given target.

As such, header files are described slightly differently than implementation
files like ``tutorial.cxx``. They're also going to need different
:ref:`scope keywords <Target Command Scope>` than the ``PRIVATE`` keyword we
have used so far.

To describe a collection of header files, we're going to use what's known as a
``FILE_SET``.

.. code-block:: cmake

  target_sources(MyLibrary
    PRIVATE
      library_implementation.cxx

    PUBLIC
      FILE_SET myHeaders
      TYPE HEADERS
      BASE_DIRS
        include
      FILES
        include/library_header.h
  )

This is a lot of complexity, but we'll go through it point by point. First,
note that we have our implementation file as a ``PRIVATE`` source, same as
with the executable previously. However, we now use ``PUBLIC`` for our
header file. This allows consumers of our library to "see" the library's
header files.

.. note::
  We're not quite ready to discuss the full semantics of scope keywords. We'll
  cover them more completely in Exercise 3.

Following the scope keyword is a ``FILE_SET``, a collection of files to be
described as a single unit. A ``FILE_SET`` consists of the following parts:

* ``FILE_SET <name>`` is the name of the ``FILE_SET``. This is a handle which
  we can use to describe the collection in other contexts.

* ``TYPE <type>`` is the kind of files we are describing. Most commonly this
  will be headers, but newer versions of CMake support other types like C++20
  modules.

* ``BASE_DIRS`` is the "base" locations for the files. This can be most easily
  understood as the locations that will be described to compilers for header
  discovery via ``-I`` flags.

* ``FILES`` is the list of files, same as with the implementation sources list
  earlier.

This is a lot of information to describe, so there are some useful shortcuts
we can take. Notably, if the ``FILE_SET`` name is the same as the type, we
don't need to provide the ``TYPE`` field.

.. code-block:: cmake

  target_sources(MyLibrary
    PRIVATE
      library_implementation.cxx

    PUBLIC
      FILE_SET HEADERS
      BASE_DIRS
        include
      FILES
        include/library_header.h
  )

There are other shortcuts we can take, but we'll discuss those more in later
steps.

Goal
----

Build a library.

Helpful Resources
-----------------

* :command:`add_library`
* :command:`target_sources`

Files to Edit
-------------

* ``CMakeLists.txt``

Getting Started
---------------

Continue editing files in the ``Step1`` directory. Start with ``TODO 5`` and
complete through ``TODO 6``.

Build and Run
-------------

Let's build our project again. Since we already created a build directory and
ran CMake for Exercise 1, we can skip to the build step:

.. code-block:: console

  cmake --build build

We should be able to see our library created alongside the Tutorial executable.

Solution
--------

We start by adding the library target in the same manner as the the Tutorial
executable.

.. raw:: html

  <details><summary>TODO 5: Click to show/hide answer</summary>

.. literalinclude:: Step3/MathFunctions/CMakeLists.txt
  :caption: TODO 5: CMakeLists.txt
  :name: CMakeLists.txt-add_library
  :language: cmake
  :start-at: add_library
  :end-at: add_library

.. raw:: html

  </details>

Next we need to describe the source files. For the implementation file,
``MathFunctions.cxx``, this is straight-forward; for the header file
``MathFunctions.h`` we will need to use a ``FILE_SET``.

We can either give this ``FILE_SET`` its own name, or use the shortcut of naming
it ``HEADERS``. For this tutorial, we'll be using the shortcut, but either
solution is valid.

For ``BASE_DIRS`` we need to determine the directory which will allow for the
desired ``#include <MathFunctions.h>`` directive. To achieve this, the
``MathFunctions`` folder itself will be a base directory. We would make a
different choice if the desired include directive were
``#include <MathFunctions/MathFunctions.h>`` or similar.

.. raw:: html

  <details><summary>TODO 6: Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 6: CMakeLists.txt
  :name: CMakeLists.txt-library_sources

  target_sources(MathFunctions
    PRIVATE
      MathFunctions/MathFunctions.cxx

    PUBLIC
      FILE_SET HEADERS
      BASE_DIRS
        MathFunctions
      FILES
        MathFunctions/MathFunctions.h
  )

.. raw:: html

  </details>

Exercise 3 - Linking Together Libraries and Executables
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

We're ready to combine our library with our executable, for this we must
introduce a new command, :command:`target_link_libraries`. The name of this
command can be somewhat misleading, as it does a great deal more than just
invoke linkers. It describes relationships between targets generally.

.. code-block:: cmake

  target_link_libraries(MyProgram
    PRIVATE
      MyLibrary
  )

We're finally ready to discuss the :ref:`scope keywords <Target Command Scope>`.
There are three of them, ``PRIVATE``, ``INTERFACE``, and ``PUBLIC``. These
describe how properties are made available to targets.

* A ``PRIVATE`` property (also called a "non-interface" property) is only
  available to the target which owns it, for example ``PRIVATE`` headers will
  only be visible to the target they're attached to.

* An ``INTERFACE`` property is only available to targets *which link* the
  owning target. The owning target does not have access to these properties. A
  header-only library is an example of a collection of ``INTERFACE`` properties,
  as header-only libraries do not build anything themselves and do not need to
  access their own files.

* ``PUBLIC`` is not a distinct kind of property, but rather is the union of the
  ``PRIVATE`` and ``INTERFACE`` properties. Thus requirements described with
  ``PUBLIC`` are available to both the owning target and consuming targets.

Consider the following concrete example:

.. code-block:: cmake

  target_sources(MyLibrary
    PRIVATE
      FILE_SET internalOnlyHeaders
      TYPE HEADERS
      FILES
        InternalOnlyHeader.h

    INTERFACE
      FILE_SET consumerOnlyHeaders
      TYPE HEADERS
      FILES
        ConsumerOnlyHeader.h

    PUBLIC
      FILE_SET publicHeaders
      TYPE HEADERS
      FILES
        PublicHeader.h
  )

.. note::
  We excluded ``BASE_DIRS`` for each file set here, that's another shortcut.
  When excluded, ``BASE_DIRS`` defaults to the current source directory.

The ``MyLibrary`` target has several properties which will be modified by this
call to :command:`target_sources`. Until now we've used the term "properties"
generically, but properties are themselves named values we can reason about.
Two specific properties which will be modified here are :prop_tgt:`HEADER_SETS`
and :prop_tgt:`INTERFACE_HEADER_SETS`, which both contain lists of header file
sets added via :command:`target_sources`.

The value ``internalOnlyHeaders`` will be added to :prop_tgt:`HEADER_SETS`,
``consumerOnlyHeaders`` to :prop_tgt:`INTERFACE_HEADER_SETS`, and
``publicHeaders`` will be added to both.

When a given target is being built, it will use its own *non-interface*
properties (eg, :prop_tgt:`HEADER_SETS`), combined with the *interface*
properties of any targets it links to (eg, :prop_tgt:`INTERFACE_HEADER_SETS`).

.. note::
  **It is not necessary to reason about CMake properties at this level of
  detail.** The above is described for completeness. Most of the time you don't
  need to be concerned with the specific properties a command is modifying.

  Scope keywords have a simple intuition associated with them, when considering
  a command from the point of view of the target it is being applied to:
  **PRIVATE** is for me, **INTERFACE** is for others, **PUBLIC** is for all of
  us.

Goal
----

In the Tutorial executable, use the ``sqrt()`` function provided by the
``MathFunctions`` library.

Helpful Resources
-----------------

* :command:`target_link_libraries`

Files to Edit
-------------

* ``CMakeLists.txt``
* ``Tutorial/Tutorial.cxx``

Getting Started
---------------

Continue to edit files from ``Step1``. Start on ``TODO 7`` and complete through
``TODO 9``. In this exercise, we need to add the ``MathFunctions`` target to
the ``Tutorial`` target's linked libraries using :command:`target_link_libraries`.

After modifying the CML, update ``tutorial.cxx`` to use the
``mathfunctions::sqrt()`` function instead of ``std::sqrt``.

Build and Run
-------------

Let's build our project again. As before, we already created a build directory
and ran CMake so we can skip to the build step:

.. code-block:: console

  cmake --build build

Verify that the output matches what you would expect from the ``MathFunctions``
library.

Solution
--------

In this exercise, we are describing the ``Tutorial`` executable as a consumer
of the ``MathFunctions`` target by adding ``MathFunctions`` to the linked
libraries of the ``Tutorial``.

To achieve this, we modify ``CMakeLists.txt`` file to use the
:command:`target_link_libraries` command, using ``Tutorial`` as the target to
be modified and ``MathFunctions`` as the library we want to add.

.. raw:: html

  <details><summary>TODO 7: Click to show/hide answer</summary>

.. literalinclude:: Step3/Tutorial/CMakeLists.txt
  :caption: TODO 7: CMakeLists.txt
  :name: CMakeLists.txt-target_link_libraries
  :language: cmake
  :start-at: target_link_libraries(Tutorial
  :end-at: )

.. raw:: html

  </details>

.. note::
  The order here is only loosely relevant. That we call
  :command:`target_link_libraries` prior to defining ``MathFunctions`` with
  :command:`add_library` doesn't matter to CMake. We are recording that
  ``Tutorial`` has a dependency on something named ``MathFunctions``, but what
  ``MathFunctions`` means isn't resolved at this stage.

  The only target which needs to be defined when calling a CMake command like
  :command:`target_sources` or :command:`target_link_libraries` is the target
  being modified.

Finally, all that's left to do is modify ``Tutorial.cxx`` to use the newly
provided ``mathfunctions::sqrt`` function. That means adding the appropriate
header file and modifying our ``sqrt()`` call.

.. raw:: html

  <details><summary>TODO 8-9: Click to show/hide answer</summary>

.. literalinclude:: Step3/Tutorial/Tutorial.cxx
  :caption: TODO 8: Tutorial/Tutorial.cxx
  :name: Tutorial/Tutorial.cxx-MathFunctions-headers
  :language: c++
  :start-at: iostream
  :end-at: MathFunctions.h

.. literalinclude:: Step3/Tutorial/Tutorial.cxx
  :caption: TODO 9: Tutorial/Tutorial.cxx
  :name: Tutorial/Tutorial.cxx-MathFunctions-code
  :language: c++
  :start-at: calculate square root
  :end-at: mathfunctions::sqrt
  :dedent: 2

.. raw:: html

  </details>

Exercise 4 - Subdirectories
^^^^^^^^^^^^^^^^^^^^^^^^^^^

As we move through the tutorial, we will be adding more commands to manipulate
the ``Tutorial`` executable and the ``MathFunctions`` library. We want to make
sure we keep commands local to the files they are dealing with. While not a
major concern for a small project like this, it can be very useful for large
projects with many targets and thousands of files.

The :command:`add_subdirectory` command allows us to incorporate CMLs located
in subdirectories of the project.

.. code-block:: cmake

  add_subdirectory(SubdirectoryName)

When a ``CMakeLists.txt`` in a subdirectory is being processed by CMake all
relative paths described in the subdirectory CML are relative to that
subdirectory, not the top-level CML.

Goal
----

Use :command:`add_subdirectory` to organize the project.

Helpful Resources
-----------------

* :command:`add_subdirectory`

Files to Edit
-------------

* ``CMakeLists.txt``
* ``Tutorial/CMakeLists.txt``
* ``MathFunctions/CMakeLists.txt``

Getting Started
---------------

The ``TODOs`` for this step are spread across three ``CMakeLists.txt`` files.
Be sure to pay attention to the path changes necessary when moving the
:command:`target_sources` commands into subdirectories.

.. note::
  Previously we said that ``BASE_DIRS`` defaults to the current source
  directory. As the desired include directory for ``MathFunctions`` will now be
  the same directory as the CML calling :command:`target_sources`, we should
  remove the ``BASE_DIRS`` keyword and argument entirely.

Complete ``TODO 10`` through ``TODO 13``.

Build and Run
-------------

Because of the reorganization, we'll need to clean the original build
directory prior to rebuilding (otherwise our new ``Target`` build folder would
conflict with our previously created ``Target`` executable). We can achieve
this with the :option:`--clean-first <cmake--build --clean-first>` flag.

There's no need for a reconfiguration. CMake will automatically
re-configure itself due to the changes in the CMLs.

.. code-block:: console

  cmake --build build --clean-first

.. note::
  Our executable and library will be output to a new location in the build tree.
  A subdirectory which mirrors where :command:`add_executable` and
  :command:`add_library` were called in the source tree. You will need to
  navigate to this subdirectory in the build tree to run the tutorial
  executable in future steps.

  You can verify this behavior by deleting the old ``Tutorial`` executable,
  and observing that the new one is produced at ``Tutorial/Tutorial``.

Solution
--------

We need to move all the commands concerning the ``Tutorial`` executable into
``Tutorial/CMakeLists.txt``, and replace them with an
:command:`add_subdirectory` command. We also need to update the path for
``Tutorial.cxx``.

.. raw:: html

  <details><summary>TODO 10-11: Click to show/hide answer</summary>

.. literalinclude:: Step3/Tutorial/CMakeLists.txt
  :caption: TODO 10: Tutorial/CMakeLists.txt
  :name: Tutorial/CMakeLists.txt-moved
  :language: cmake

.. code-block:: cmake
  :caption: TODO 11: CMakeLists.txt
  :name: CMakeLists.txt-add_subdirectory-Tutorial

  add_subdirectory(Tutorial)

.. raw:: html

  </details>

We need to do the same with the commands for ``MathFunctions``, changing the
relative paths as appropriate and removing ``BASE_DIRS`` as it is no longer
necessary, the default value will work.

.. raw:: html

  <details><summary>TODO 12-13: Click to show/hide answer</summary>

.. literalinclude:: Step3/MathFunctions/CMakeLists.txt
  :caption: TODO 12: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-moved
  :language: cmake

.. literalinclude:: Step3/CMakeLists.txt
  :caption: TODO 13: CMakeLists.txt
  :name: CMakeLists.txt-add_subdirectory-MathFunctions
  :language: cmake
  :start-at: add_subdirectory(MathFunctions
  :end-at: add_subdirectory(MathFunctions

.. raw:: html

  </details>
