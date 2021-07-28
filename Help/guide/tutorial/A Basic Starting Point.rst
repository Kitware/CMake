Step 1: A Basic Starting Point
==============================

The most basic project is an executable built from source code files.
For simple projects, a three line ``CMakeLists.txt`` file is all that is
required. This will be the starting point for our tutorial. Create a
``CMakeLists.txt`` file in the ``Step1`` directory that looks like:

.. code-block:: cmake
  :caption: CMakeLists.txt
  :name: CMakeLists.txt-start

  cmake_minimum_required(VERSION 3.10)

  # set the project name
  project(Tutorial)

  # add the executable
  add_executable(Tutorial tutorial.cxx)


Note that this example uses lower case commands in the ``CMakeLists.txt`` file.
Upper, lower, and mixed case commands are supported by CMake. The source
code for ``tutorial.cxx`` is provided in the ``Step1`` directory and can be
used to compute the square root of a number.

Adding a Version Number and Configured Header File
--------------------------------------------------

The first feature we will add is to provide our executable and project with a
version number. While we could do this exclusively in the source code, using
``CMakeLists.txt`` provides more flexibility.

First, modify the ``CMakeLists.txt`` file to use the :command:`project` command
to set the project name and version number.

.. literalinclude:: Step2/CMakeLists.txt
  :caption: CMakeLists.txt
  :name: CMakeLists.txt-project-VERSION
  :language: cmake
  :end-before: # specify the C++ standard

Then, configure a header file to pass the version number to the source
code:

.. literalinclude:: Step2/CMakeLists.txt
  :caption: CMakeLists.txt
  :name: CMakeLists.txt-configure_file
  :language: cmake
  :start-after: # to the source code
  :end-before: # add the executable

Since the configured file will be written into the binary tree, we
must add that directory to the list of paths to search for include
files. Add the following lines to the end of the ``CMakeLists.txt`` file:

.. literalinclude:: Step2/CMakeLists.txt
  :caption: CMakeLists.txt
  :name: CMakeLists.txt-target_include_directories
  :language: cmake
  :start-after: # so that we will find TutorialConfig.h

Using your favorite editor, create ``TutorialConfig.h.in`` in the source
directory with the following contents:

.. literalinclude:: Step2/TutorialConfig.h.in
  :caption: TutorialConfig.h.in
  :name: TutorialConfig.h.in
  :language: c++

When CMake configures this header file the values for
``@Tutorial_VERSION_MAJOR@`` and ``@Tutorial_VERSION_MINOR@`` will be
replaced.

Next modify ``tutorial.cxx`` to include the configured header file,
``TutorialConfig.h``.

Finally, let's print out the executable name and version number by updating
``tutorial.cxx`` as follows:

.. literalinclude:: Step2/tutorial.cxx
  :caption: tutorial.cxx
  :name: tutorial.cxx-print-version
  :language: c++
  :start-after: {
  :end-before: // convert input to double

Specify the C++ Standard
-------------------------

Next let's add some C++11 features to our project by replacing ``atof`` with
``std::stod`` in ``tutorial.cxx``.  At the same time, remove
``#include <cstdlib>``.

.. literalinclude:: Step2/tutorial.cxx
  :caption: tutorial.cxx
  :name: tutorial.cxx-cxx11
  :language: c++
  :start-after: // convert input to double
  :end-before: // calculate square root

We will need to explicitly state in the CMake code that it should use the
correct flags. The easiest way to enable support for a specific C++ standard
in CMake is by using the :variable:`CMAKE_CXX_STANDARD` variable. For this
tutorial, set the :variable:`CMAKE_CXX_STANDARD` variable in the
``CMakeLists.txt`` file to ``11`` and :variable:`CMAKE_CXX_STANDARD_REQUIRED`
to ``True``. Make sure to add the ``CMAKE_CXX_STANDARD`` declarations above the
call to ``add_executable``.

.. literalinclude:: Step2/CMakeLists.txt
  :caption: CMakeLists.txt
  :name: CMakeLists.txt-CXX_STANDARD
  :language: cmake
  :end-before: # configure a header file to pass some of the CMake settings

Build and Test
--------------

Run the :manual:`cmake <cmake(1)>` executable or the
:manual:`cmake-gui <cmake-gui(1)>` to configure the project and then build it
with your chosen build tool.

For example, from the command line we could navigate to the
``Help/guide/tutorial`` directory of the CMake source code tree and create a
build directory:

.. code-block:: console

  mkdir Step1_build

Next, navigate to the build directory and run CMake to configure the project
and generate a native build system:

.. code-block:: console

  cd Step1_build
  cmake ../Step1

Then call that build system to actually compile/link the project:

.. code-block:: console

  cmake --build .

Finally, try to use the newly built ``Tutorial`` with these commands:

.. code-block:: console

  Tutorial 4294967296
  Tutorial 10
  Tutorial
