CMake Debugging Guide
*********************

This guide explains how to attach a debugger to CMake's unit testing framework.
We'll focus on using **GDB** on Linux for both command-line and IDE debugging.
See documentation on `CMake Development`_ for more information.

.. _`CMake Development`: README.rst

Linux: Using GDB
================

On Linux, the GNU Debugger (**GDB**) is the standard tool for debugging the
CMake test suite. The core process involves launching the ``cmake`` executable
from within GDB with a specific set of arguments that configure and run the
desired test.

GDB Configuration
-----------------

For effective debugging, GDB must be configured to handle child processes
correctly, which CMake tests often create. A good practice is to use a local
``.gdbinit`` file in your build directory. This keeps CMake-specific settings
separate from your global configuration.

**1. Enable Local .gdbinit Files (One-Time Setup)**

To allow GDB to automatically load configuration from your build directory,
add the following line to your global GDB initialization file at
``$HOME/.gdbinit``. This is a one-time setup that makes future projects easier
to manage.

.. code-block:: text

  set auto-load local-gdbinit on

**2. Create a Project-Specific .gdbinit**

Next, create a ``.gdbinit`` file inside your CMake **build directory**.
This file will contain settings specific to debugging CMake.
To make this easier, you can symlink the template file provided in the CMake
source tree:

.. code-block:: bash

  # Navigate to your build directory
  cd /path/to/your/cmake/build

  # Create a symlink to the template
  ln -s $cmake_srcdir/Utilities/gdb/gdbinit-template .gdbinit

The template contains the essential settings for debugging CMake tests:

.. code-block:: gdb

  # Allows GDB to follow child processes
  set follow-fork-mode child

  # Allows the parent process continue in parallel
  set non-stop on

Debugging from the Command Line
-------------------------------

To start debugging, first cd to the build directory. Then, launch the
``cmake`` executable using ``gdb --args``, which passes the necessary test
configuration arguments directly to CMake.

.. note::

  To get the launch command, run ``ctest -R "RunCMake.$TESTNAME" -VV -N``


The following example runs the ``InstallPackageInfo`` test.

.. code-block:: bash

  # Define paths to your CMake source and build directories
  CMAKE_SOURCE_DIR="$HOME/cmake"
  CMAKE_BUILD_DIR="$CMAKE_SOURCE_DIR/build"

  # Define the specific test to run
  TEST_NAME="InstallPackageInfo"

  # Navigate to the build directory
  cd "$CMAKE_BUILD_DIR"

  # Launch GDB with the appropriate arguments for the test
  gdb --args ./bin/cmake \
    "-DCMAKE_MODULE_PATH=$CMAKE_SOURCE_DIR/Tests/RunCMake" \
    "-DRunCMake_GENERATOR=Ninja" \
    "-DRunCMake_SOURCE_DIR=$CMAKE_SOURCE_DIR/Tests/RunCMake/$TEST_NAME" \
    "-DRunCMake_BINARY_DIR=$CMAKE_BUILD_DIR/Tests/RunCMake/$TEST_NAME" \
    "-P" "$CMAKE_SOURCE_DIR/Tests/RunCMake/RunCMakeTest.cmake"

Once GDB loads, you may set breakpoints (e.g., ``b cmInstallCommand``) and
then start the test by typing ``run``.

Filtering Tests
---------------

Some test suites contain multiple sub-tests. To run only a specific one,
you can use the ``RunCMake_TEST_FILTER`` environment variable.

For example, to run only the "Metadata" test within the ``InstallPackageInfo``
suite, you can set the variable before launching GDB:

.. code-block:: bash

  RunCMake_TEST_FILTER="Metadata" gdb --args ...

Alternatively, you can set the environment variable from within the
GDB session before running the test:

.. code-block:: gdb-prompt

  (gdb) set environment RunCMake_TEST_FILTER Metadata
  (gdb) run


IDE Integration
---------------

You can also debug CMake tests directly from your IDE.

CLion
=====

If you have configured GDB to auto-load local ``.gdbinit`` files as described
above, CLion will automatically pick up the necessary settings.

A simple way to debug a test is to modify its ``CTest`` run configuration:

#. **Select the Test**: In the "Run/Debug Configurations" dialog, find the
   ``CTest`` entry for your test (e.g., ``RunCMake.InstallPackageInfo``).
#. **Add CTest Arguments**: In the "CTest arguments" field, add
   ``--extra-verbose``. This is helpful for debugging because it prints the
   exact command ``CTest`` uses to run the test.
#. **Set Working Directory**: Ensure the "Working Directory" field is set to
   ``$CMakeCurrentLocalGenerationDir$``.

You can now set breakpoints in your code and debug this configuration.

Visual Studio Code
==================

Create a ``launch.json`` file in the ``.vscode`` directory of your
CMake **source folder** with the following configuration. This configuration
hardcodes the necessary GDB settings, so it does not depend on an external
``.gdbinit`` file.

.. code-block:: json

  {
   "version": "0.2.0",
   "configurations": [
     {
       "name": "Debug CMake Test",
       "type": "cppdbg",
       "request": "launch",
       "program": "${workspaceFolder}/build/bin/cmake",
       "args": [
         "-DCMAKE_MODULE_PATH=${workspaceFolder}/Tests/RunCMake",
         "-DRunCMake_GENERATOR=Ninja",
         "-DRunCMake_SOURCE_DIR=${workspaceFolder}/Tests/RunCMake/InstallPackageInfo",
         "-DRunCMake_BINARY_DIR=${workspaceFolder}/build/Tests/RunCMake/InstallPackageInfo",
         "-P",
         "${workspaceFolder}/Tests/RunCMake/RunCMakeTest.cmake"
       ],
       "stopAtEntry": false,
       "cwd": "${workspaceFolder}/build",
       "environment": [],
       "MIMode": "gdb",
       "setupCommands": [
         {
           "description": "Enable pretty-printing for gdb",
           "text": "-enable-pretty-printing",
           "ignoreFailures": true
         },
         {
           "description": "Follow child processes",
           "text": "set follow-fork-mode child",
           "ignoreFailures": true
         },
         {
           "description": "Don't stop the parent process",
           "text": "set non-stop on",
           "ignoreFailures": true
         }
       ]
     }
   ]
  }

.. note::

  Remember to change the test name (``InstallPackageInfo``) in the ``"args"`` section to the specific test you want to debug.
