Step 6: Adding a Custom Command and Generated File
==================================================

Suppose, for the purpose of this tutorial, we decide that we never want to use
the platform ``log`` and ``exp`` functions and instead would like to
generate a table of precomputed values to use in the ``mysqrt`` function.
In this section, we will create the table as part of the build process,
and then compile that table into our application.

First, let's remove the check for the ``log`` and ``exp`` functions in
``MathFunctions/CMakeLists.txt``. Then remove the check for ``HAVE_LOG`` and
``HAVE_EXP`` from ``mysqrt.cxx``. At the same time, we can remove
:code:`#include <cmath>`.

In the ``MathFunctions`` subdirectory, a new source file named
``MakeTable.cxx`` has been provided to generate the table.

After reviewing the file, we can see that the table is produced as valid C++
code and that the output filename is passed in as an argument.

The next step is to add the appropriate commands to the
``MathFunctions/CMakeLists.txt`` file to build the MakeTable executable and
then run it as part of the build process. A few commands are needed to
accomplish this.

First, at the top of ``MathFunctions/CMakeLists.txt``, the executable for
``MakeTable`` is added as any other executable would be added.

.. literalinclude:: Step7/MathFunctions/CMakeLists.txt
  :caption: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-add_executable-MakeTable
  :language: cmake
  :start-after: # first we add the executable that generates the table
  :end-before: # add the command to generate the source code

Then we add a custom command that specifies how to produce ``Table.h``
by running MakeTable.

.. literalinclude:: Step7/MathFunctions/CMakeLists.txt
  :caption: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-add_custom_command-Table.h
  :language: cmake
  :start-after: # add the command to generate the source code
  :end-before: # add the main library

Next we have to let CMake know that ``mysqrt.cxx`` depends on the generated
file ``Table.h``. This is done by adding the generated ``Table.h`` to the list
of sources for the library MathFunctions.

.. literalinclude:: Step7/MathFunctions/CMakeLists.txt
  :caption: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-add_library-Table.h
  :language: cmake
  :start-after: # add the main library
  :end-before: # state that anybody linking

We also have to add the current binary directory to the list of include
directories so that ``Table.h`` can be found and included by ``mysqrt.cxx``.

.. literalinclude:: Step7/MathFunctions/CMakeLists.txt
  :caption: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-target_include_directories-Table.h
  :language: cmake
  :start-after: # state that we depend on our bin
  :end-before: # install rules

Now let's use the generated table. First, modify ``mysqrt.cxx`` to include
``Table.h``. Next, we can rewrite the ``mysqrt`` function to use the table:

.. literalinclude:: Step7/MathFunctions/mysqrt.cxx
  :caption: MathFunctions/mysqrt.cxx
  :name: MathFunctions/mysqrt.cxx
  :language: c++
  :start-after: // a hack square root calculation using simple operations

Run the :manual:`cmake  <cmake(1)>` executable or the
:manual:`cmake-gui <cmake-gui(1)>` to configure the project and then build it
with your chosen build tool.

When this project is built it will first build the ``MakeTable`` executable.
It will then run ``MakeTable`` to produce ``Table.h``. Finally, it will
compile ``mysqrt.cxx`` which includes ``Table.h`` to produce the
``MathFunctions`` library.

Run the Tutorial executable and verify that it is using the table.
