Step 7: Custom Commands and Generated Files
===========================================

Code generation is a ubiquitous mechanism for extending programming languages
beyond the bounds of their language model. CMake has first-class support for
Qt's Meta-Object Compiler, but very few other code generators are notable
enough to warrant that kind of effort.

Instead, code generators tend to be bespoke and usage specific. CMake provides
facilities for describing the usage of a code generator, so projects can
add support for their individual needs.

In this step, we will use :command:`add_custom_command` to add support for a
code generator within the tutorial project.

Background
^^^^^^^^^^

Any step in the build process can generally be described in terms of its inputs
and outputs. CMake assumes that code generators and other custom processes
operate on the same principle. In this way, the code generator acts identically
to compilers, linkers, and other elements of the toolchain; when the inputs are
newer than the outputs (or the outputs don't exist), a user-specified command
will be run to update the outputs.

.. note::
  This model assumes the outputs of a process are known before it is run. CMake
  lacks the ability to describe code generators where the name and location of
  the outputs depends on the *content* of the input. Various hacks exist to
  shim this functionality into CMake, but they are outside the scope of this
  tutorial.

Describing a code generator (or any custom process) is usually performed in
two parts. First, the inputs and outputs are described independently of the
CMake target model, concerned only with the generation process itself. Second,
the outputs are associated with a CMake target to insert them into the CMake
target model.

For sources, this is as simple as adding the generated files to the source list
of a ``STATIC``, ``SHARED``, or ``OBJECT`` library. For header-only generators,
it's often necessary to use an intermediary target created via
:command:`add_custom_target` to add the header file generation to the
build stage (because ``INTERFACE`` libraries have no build step).

Exercise 1 - Using a Code Generator
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The primary mechanism for describing a code generator is the
:command:`add_custom_command` command. A "command", for the purpose of
:command:`add_custom_command` is either an executable available in the build
environment or a CMake executable target name.

.. code-block:: cmake

  add_executable(Tool)
  # ...
  add_custom_command(
    OUTPUT Generated.cxx
    COMMAND Tool -i input.txt -o Generated.cxx
    DEPENDS Tool input.txt
    VERBATIM
  )
  # ...
  add_library(GeneratedObject OBJECT)
  target_sources(GeneratedObject
    PRIVATE
      Generated.cxx
  )

Most of the keywords are self-explanatory, with the exception of ``VERBATIM``.
This argument is effectively mandatory for legacy reasons that are uninteresting
to explain in a modern context. The curious should consult the
:command:`add_custom_command` documentation for additional details.

The ``Tool`` executable target appears both in the ``COMMAND`` and ``DEPENDS``
parameters. While ``COMMAND`` is sufficient for the code to build correctly,
adding the ``Tool`` itself as a dependency of the custom command ensure that
if ``Tool`` is updated, the custom command will be rerun.

For header-only file generation, additional commands are necessary because the
library itself has no build step. We can use :command:`add_custom_target` to
create an "artificial" build step for the library. We then force the custom
target to be run before any targets which link the library with the command
:command:`add_dependencies`.

.. code-block:: cmake

  add_custom_target(RunGenerator DEPENDS Generated.h)

  add_library(GeneratedLib INTERFACE)
  target_sources(GeneratedLib
    INTERFACE
      FILE_SET HEADERS
      BASE_DIRS
        ${CMAKE_CURRENT_BINARY_DIR}
      FILES
        ${CMAKE_CURRENT_BINARY_DIR}/Generated.h
  )

  add_dependencies(GeneratedLib RunGenerator)

.. note::
  We add the :variable:`CMAKE_CURRENT_BINARY_DIR`, a variable which names the
  current location in the build tree where our artifacts are being placed, to
  the base directories because that's the working directory our code generator
  will be run inside of. Listing the ``FILES`` is unnecessary for the build and
  done so here only for clarity.

Goal
----

Add a generated table of pre-computed square roots to the ``MathFunctions``
library.

Helpful Resources
-----------------

* :command:`add_executable`
* :command:`add_library`
* :command:`target_sources`
* :command:`add_custom_command`
* :command:`add_custom_target`
* :command:`add_dependencies`

Files to Edit
-------------

* ``MathFunctions/CMakeLists.txt``
* ``MathFunctions/MakeTable/CMakeLists.txt``
* ``MathFunctions/MathFunctions.cxx``

Getting Started
---------------

The ``MathFunctions`` library has been edited to use a pre-computed table when
given a number less than 10. However, the hardcoded table is not particularly
accurate, containing only the nearest truncated integer value.

The ``MakeTable.cxx`` source file describes a program which will generate a
better table. It takes a single argument as input, the file name of the table
to be generated.

Complete ``TODO 1`` through ``TODO 10``.

Build and Run
-------------

No special configuration is needed, configure and build as usual. Note that
the ``MakeTable`` executable is sequenced before ``MathFunctions``.

.. code-block:: console

  cmake --preset tutorial
  cmake --build build

Verify the output of ``Tutorial`` now uses the pre-computed table for values
less than 10.

Solution
--------

First we add a new executable to generate the tables, adding the
``MakeTable.cxx`` file as a source.

.. raw:: html

  <details><summary>TODO 1-2: Click to show/hide answer</summary>

.. literalinclude:: Step8/MathFunctions/MakeTable/CMakeLists.txt
  :caption: TODO 1-2: MathFunctions/MakeTable/CMakeLists.txt
  :name: MathFunctions/MakeTable/CMakeLists.txt-add_executable
  :language: cmake
  :start-at: add_executable
  :end-at: MakeTable.cxx
  :append: )

.. raw:: html

  </details>

Then we add a custom command which produces the table, and custom target which
depends on the table.

.. raw:: html

  <details><summary>TODO 3-4: Click to show/hide answer</summary>

.. literalinclude:: Step8/MathFunctions/MakeTable/CMakeLists.txt
  :caption: TODO 3-4: MathFunctions/MakeTable/CMakeLists.txt
  :name: MathFunctions/MakeTable/CMakeLists.txt-add_custom_command
  :language: cmake
  :start-at: add_custom_command
  :end-at: add_custom_target

.. raw:: html

  </details>

We need to add an interface library which describes the output which will
appear in :variable:`CMAKE_CURRENT_BINARY_DIR`. The ``FILES`` parameter is
optional.

.. raw:: html

  <details><summary>TODO 5-6: Click to show/hide answer</summary>

.. literalinclude:: Step8/MathFunctions/MakeTable/CMakeLists.txt
  :caption: TODO 5-6: MathFunctions/MakeTable/CMakeLists.txt
  :name: MathFunctions/MakeTable/CMakeLists.txt-add_library
  :language: cmake
  :start-at: add_library
  :end-at: SqrtTable.h
  :append: )

.. raw:: html

  </details>

Now that all the targets are described, we can force the custom target to run
before any dependents of the interface library by associating them with
:command:`add_dependencies`.

.. raw:: html

  <details><summary>TODO 7: Click to show/hide answer</summary>

.. literalinclude:: Step8/MathFunctions/MakeTable/CMakeLists.txt
  :caption: TODO 7: MathFunctions/MakeTable/CMakeLists.txt
  :name: MathFunctions/MakeTable/CMakeLists.txt-add_dependencies
  :language: cmake
  :start-at: add_dependencies
  :end-at: add_dependencies

.. raw:: html

  </details>

We are ready to add the interface library to the linked libraries of
``MathFunctions``, and add the entire ``MakeTable`` folder to the project.

.. raw:: html

  <details><summary>TODO 8-9: Click to show/hide answer</summary>

.. literalinclude:: Step8/MathFunctions/CMakeLists.txt
  :caption: TODO 8: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-link-sqrttable
  :language: cmake
  :start-at: target_link_libraries(MathFunctions
  :end-at: )

.. literalinclude:: Step8/MathFunctions/CMakeLists.txt
  :caption: TODO 9: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-add-maketable
  :language: cmake
  :start-at: add_subdirectory(MakeTable
  :end-at: add_subdirectory(MakeTable

.. raw:: html

  </details>

Finally, we update the ``MathFunctions`` library itself to take advantage of
the generated table.

.. raw:: html

  <details><summary>TODO 10: Click to show/hide answer</summary>

.. literalinclude:: Step8/MathFunctions/MathFunctions.cxx
  :caption: TODO 10: MathFunctions/MathFunctions.cxx
  :name: MathFunctions/MathFunctions.cxx-include-sqrttable
  :language: c++
  :start-at: #include <SqrtTable.h>
  :end-at: {

.. raw:: html

  </details>
