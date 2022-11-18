Step 5: Installing and Testing
==============================

Exercise 1 - Install Rules
^^^^^^^^^^^^^^^^^^^^^^^^^^

Often, it is not enough to only build an executable, it should also be
installable. With CMake, we can specify install rules using the
:command:`install` command. Supporting local installations for your builds in
CMake is often as simple as specifying an install location and the targets and
files to be installed.

Goal
----

Install the ``Tutorial`` executable and the ``MathFunctions`` library.

Helpful Materials
-----------------

* :command:`install`

Files to Edit
-------------

* ``MathFunctions/CMakeLists.txt``
* ``CMakeLists.txt``

Getting Started
---------------

The starting code is provided in the ``Step5`` directory. In this
exercise, complete ``TODO 1`` through ``TODO 4``.

First, update ``MathFunctions/CMakeLists.txt`` to install the
``MathFunctions`` and ``tutorial_compiler_flags`` libraries to the ``lib``
directory. In that same file, specify the install rules needed to install
``MathFunctions.h`` to the ``include`` directory.

Then, update the top level ``CMakeLists.txt`` to install
the ``Tutorial`` executable to the ``bin`` directory. Lastly, any header files
should be installed to the ``include`` directory. Remember that
``TutorialConfig.h`` is in the :variable:`PROJECT_BINARY_DIR`.

Build and Run
-------------

Make a new directory called ``Step5_build``. Run the
:manual:`cmake <cmake(1)>` executable or the
:manual:`cmake-gui <cmake-gui(1)>` to configure the project and then build it
with your chosen build tool.

Then, run the install step by using the :option:`--install <cmake --install>`
option of the :manual:`cmake  <cmake(1)>` command (introduced in 3.15, older
versions of CMake must use ``make install``) from the command line. This step
will install the appropriate header files, libraries, and executables.
For example:

.. code-block:: console

  cmake --install .

For multi-configuration tools, don't forget to use the
:option:`--config <cmake--build --config>` argument to specify the configuration.

.. code-block:: console

  cmake --install . --config Release

If using an IDE, simply build the ``INSTALL`` target. You can build the same
install target from the command line like the following:

.. code-block:: console

  cmake --build . --target install --config Debug

The CMake variable :variable:`CMAKE_INSTALL_PREFIX` is used to determine the
root of where the files will be installed. If using the :option:`cmake --install`
command, the installation prefix can be overridden via the
:option:`--prefix <cmake--install --prefix>` argument. For example:

.. code-block:: console

  cmake --install . --prefix "/home/myuser/installdir"

Navigate to the install directory and verify that the installed ``Tutorial``
runs.

Solution
--------

The install rules for our project are fairly simple:

* For ``MathFunctions``, we want to install the libraries and header file to
  the ``lib`` and ``include`` directories respectively.

* For the ``Tutorial`` executable, we want to install the executable and
  configured header file to the ``bin`` and ``include`` directories
  respectively.

So to the end of ``MathFunctions/CMakeLists.txt`` we add:

.. raw:: html

  <details><summary>TODO 1: Click to show/hide answer</summary>

.. literalinclude:: Step6/MathFunctions/CMakeLists.txt
  :caption: TODO 1: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-install-TARGETS
  :language: cmake
  :start-after: # install libs
  :end-before: # install include headers

.. raw:: html

  </details>

and

.. raw:: html

  <details><summary>TODO 2: Click to show/hide answer</summary>

.. literalinclude:: Step6/MathFunctions/CMakeLists.txt
  :caption: TODO 2: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-install-headers
  :language: cmake
  :start-after: # install include headers

.. raw:: html

  </details>

The install rules for the ``Tutorial`` executable and configured header file
are similar. To the end of the top-level ``CMakeLists.txt`` we add:

.. raw:: html

  <details><summary>TODO 3,4: Click to show/hide answer</summary>

.. literalinclude:: Step6/CMakeLists.txt
  :caption: CMakeLists.txt
  :name: TODO 3,4: CMakeLists.txt-install-TARGETS
  :language: cmake
  :start-after: # add the install targets
  :end-before: # TODO 1: Replace enable_testing() with include(CTest)

.. raw:: html

  </details>

That is all that is needed to create a basic local
install of the tutorial.

.. _`Tutorial Testing Support`:

Exercise 2 - Testing Support
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

CTest offers a way to easily manage tests for your project. Tests can be
added through the :command:`add_test` command. Although it is not
explicitly covered in this tutorial, there is a lot of compatibility
between CTest and other testing frameworks such as :module:`GoogleTest`.

Goal
----

Create unit tests for our executable using CTest.

Helpful Materials
-----------------

* :command:`enable_testing`
* :command:`add_test`
* :command:`function`
* :command:`set_tests_properties`
* :manual:`ctest <ctest(1)>`

Files to Edit
-------------

* ``CMakeLists.txt``

Getting Started
---------------

The starting source code is provided in the ``Step5`` directory. In this
exercise, complete ``TODO 5`` through ``TODO 9``.

First, we need to enable testing. Next, begin adding tests to our project
using :command:`add_test`. We will work through adding 3 simple tests and
then you can add additional testing as you see fit.

Build and Run
-------------

Navigate to the build directory and rebuild the application. Then, run the
:program:`ctest` executable: :option:`ctest -N` and :option:`ctest -VV`. For
multi-config generators (e.g. Visual Studio), the configuration type must be
specified with the :option:`-C \<mode\> <ctest -C>` flag.  For example, to run tests in Debug
mode use ``ctest -C Debug -VV`` from the build directory
(not the Debug subdirectory!). Release mode would be executed from the same
location but with a ``-C Release``. Alternatively, build the ``RUN_TESTS``
target from the IDE.

Solution
--------

Let's test our application. At the end of the top-level ``CMakeLists.txt``
file we first need to enable testing with the
:command:`enable_testing` command.

.. raw:: html

  <details><summary>TODO 5: Click to show/hide answer</summary>

.. literalinclude:: Step6/CMakeLists.txt
  :caption: TODO 5: CMakeLists.txt
  :name: CMakeLists.txt-enable_testing
  :language: cmake
  :start-after: # enable testing
  :end-before: # does the application run

.. raw:: html

  </details>

With testing enabled, we will add a number of basic tests to verify
that the application is working correctly. First, we create a test using
:command:`add_test` which runs the ``Tutorial`` executable with the
parameter 25 passed in. For this test, we are not going to check the
executable's computed answer. This test will verify that
application runs, does not segfault or otherwise crash, and has a zero
return value. This is the basic form of a CTest test.

.. raw:: html

  <details><summary>TODO 6: Click to show/hide answer</summary>

.. literalinclude:: Step6/CMakeLists.txt
  :caption: TODO 6: CMakeLists.txt
  :name: CMakeLists.txt-test-runs
  :language: cmake
  :start-after: # does the application run
  :end-before: # does the usage message work

.. raw:: html

  </details>

Next, let's use the :prop_test:`PASS_REGULAR_EXPRESSION` test property to
verify that the output of the test contains certain strings. In this case,
verifying that the usage message is printed when an incorrect number of
arguments are provided.

.. raw:: html

  <details><summary>TODO 7: Click to show/hide answer</summary>

.. literalinclude:: Step6/CMakeLists.txt
  :caption: TODO 7: CMakeLists.txt
  :name: CMakeLists.txt-test-usage
  :language: cmake
  :start-after: # does the usage message work?
  :end-before: # define a function to simplify adding tests

.. raw:: html

  </details>

The next test we will add verifies the computed value is truly the
square root.

.. raw:: html

  <details><summary>TODO 8: Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 8: CMakeLists.txt
  :name: CMakeLists.txt-test-standard

  add_test(NAME StandardUse COMMAND Tutorial 4)
  set_tests_properties(StandardUse
    PROPERTIES PASS_REGULAR_EXPRESSION "4 is 2"
    )

.. raw:: html

  </details>

This one test is not enough to give us confidence that it will
work for all values passed in. We should add more tests to verify this.
To easily add more tests, we make a function called ``do_test`` that runs the
application and verifies that the computed square root is correct for
given input. For each invocation of ``do_test``, another test is added to
the project with a name, input, and expected results based on the passed
arguments.

.. raw:: html

  <details><summary>TODO 9: Click to show/hide answer</summary>

.. literalinclude:: Step6/CMakeLists.txt
  :caption: TODO 9: CMakeLists.txt
  :name: CMakeLists.txt-generalized-tests
  :language: cmake
  :start-after: # define a function to simplify adding tests

.. raw:: html

  </details>
