Step 4: Installing and Testing
==============================

Now we can start adding install rules and testing support to our project.

Install Rules
-------------

The install rules are fairly simple: for ``MathFunctions`` we want to install
the library and header file and for the application we want to install the
executable and configured header.

So to the end of ``MathFunctions/CMakeLists.txt`` we add:

.. literalinclude:: Step5/MathFunctions/CMakeLists.txt
  :caption: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-install-TARGETS
  :language: cmake
  :start-after: # install rules

And to the end of the top-level ``CMakeLists.txt`` we add:

.. literalinclude:: Step5/CMakeLists.txt
  :caption: CMakeLists.txt
  :name: CMakeLists.txt-install-TARGETS
  :language: cmake
  :start-after: # add the install targets
  :end-before: # enable testing

That is all that is needed to create a basic local install of the tutorial.

Now run the :manual:`cmake  <cmake(1)>` executable or the
:manual:`cmake-gui <cmake-gui(1)>` to configure the project and then build it
with your chosen build tool.

Then run the install step by using the ``install`` option of the
:manual:`cmake  <cmake(1)>` command (introduced in 3.15, older versions of
CMake must use ``make install``) from the command line. For
multi-configuration tools, don't forget to use the ``--config`` argument to
specify the configuration. If using an IDE, simply build the ``INSTALL``
target. This step will install the appropriate header files, libraries, and
executables. For example:

.. code-block:: console

  cmake --install .

The CMake variable :variable:`CMAKE_INSTALL_PREFIX` is used to determine the
root of where the files will be installed. If using the ``cmake --install``
command, the installation prefix can be overridden via the ``--prefix``
argument. For example:

.. code-block:: console

  cmake --install . --prefix "/home/myuser/installdir"

Navigate to the install directory and verify that the installed Tutorial runs.

.. _`Tutorial Testing Support`:

Testing Support
---------------

Next let's test our application. At the end of the top-level ``CMakeLists.txt``
file we can enable testing and then add a number of basic tests to verify that
the application is working correctly.

.. literalinclude:: Step5/CMakeLists.txt
  :caption: CMakeLists.txt
  :name: CMakeLists.txt-enable_testing
  :language: cmake
  :start-after: # enable testing

The first test simply verifies that the application runs, does not segfault or
otherwise crash, and has a zero return value. This is the basic form of a
CTest test.

The next test makes use of the :prop_test:`PASS_REGULAR_EXPRESSION` test
property to verify that the output of the test contains certain strings. In
this case, verifying that the usage message is printed when an incorrect number
of arguments are provided.

Lastly, we have a function called ``do_test`` that runs the application and
verifies that the computed square root is correct for given input. For each
invocation of ``do_test``, another test is added to the project with a name,
input, and expected results based on the passed arguments.

Rebuild the application and then cd to the binary directory and run the
:manual:`ctest <ctest(1)>` executable: ``ctest -N`` and ``ctest -VV``. For
multi-config generators (e.g. Visual Studio), the configuration type must be
specified. To run tests in Debug mode, for example, use ``ctest -C Debug -VV``
from the build directory (not the Debug subdirectory!). Alternatively, build
the ``RUN_TESTS`` target from the IDE.
