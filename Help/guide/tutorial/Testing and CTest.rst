Step 8: Testing and CTest
=========================

Testing is, historically, not the role of the build system. At best it might
have a specific target which maps to building and running the project's tests.

In the CMake ecosystem, the opposite is true. CMake's testing ecosystem is
known as CTest. This ecosystem is both deceivingly simple and incredibly
powerful. In fact it is so powerful it deserves its own full tutorial to
describe everything we could achieve with it.

This is not that tutorial. In this step, we will scratch the surface of some
of the facilities that CTest provides.

Background
^^^^^^^^^^

At its core, CTest is a task launcher which runs commands and reports if they
have returned zero or non-zero values. This is the level we will be dealing
with CTest at.

CMake provides direct integration with CTest via the :command:`enable_testing`
and :command:`add_test` commands. These allow CMake to setup the necessary
infrastructure in the build folder for CTest to discover, run, and report
on various tests we might be interested in.

After setting up and building tests, the easiest way to invoke CTest is to run
it directly on the build directory with:

.. code-block:: console

  ctest --test-dir build

Which will run all available tests. Specific tests can be run with regular
expressions.

.. code-block:: console

  ctest --test-dir build -R SpecificTest

CTest also has advanced mechanisms for scripting, fixtures, sanitizers,
job servers, metric reportings, and much more. See the :manual:`ctest(1)`
manual for more information.

Exercise 1 - Adding Tests
^^^^^^^^^^^^^^^^^^^^^^^^^

CTest convention dictates the building and running of tests be based on a
default-``ON`` variable named :variable:`BUILD_TESTING`. When using the full
suite of CTest capabilities via the :module:`CTest` module, this
:command:`option` is setup for us. When using a more stripped-down approach to
testing, it's expected the project will setup the option (or at least one of a
similar name) on its own.

When :variable:`BUILD_TESTING` is true, the :command:`enable_testing` command
should be called in the root CML.

.. code-block:: cmake

  enable_testing()

This will generate all the necessary metadata into the build tree for CTest to
find and run tests.

Once that has been done, the :command:`add_test` command can be used to create
a test anywhere in the project. The semantics of this command are similar to
:command:`add_custom_command`; we can name an executable target as the "command".

.. code-block:: cmake

  add_test(
    NAME MyAppWithTestFlag
    COMMAND MyApp --test
  )

Goal
----

Add tests for the MathFunctions library to the project and run them with CTest.

Helpful Resources
-----------------

* :variable:`BUILD_TESTING`
* :command:`enable_testing`
* :command:`function`
* :command:`add_test`

Files to Edit
-------------

* ``Tests/CMakeLists.txt``
* ``CMakeLists.txt``

Getting Started
---------------

A testing program has been written in the file ``Tests/TestMathFunctions.cxx``.
This program takes a single command line argument, the math function to be
tested, with valid values of ``add``, ``mul``, ``sqrt``, and ``sub``. The return
code is zero if the operation is recognized and the calculated value is valid,
otherwise it is non-zero.

Complete ``TODO 1`` through ``TODO 7``.

Build and Run
-------------

No special configuration is needed, configure and build as usual.

.. code-block:: console

  cmake --preset tutorial
  cmake --build build

Verify all the tests pass with CTest.

.. note::

  If using a multi-config generator, eg Visual Studio, it will be necessary to
  specify a configuration with ``ctest -C <config> <remaining flags>``, where
  ``<config>`` is a value like ``Debug`` or ``Release``. This is true whenever
  using a multi-config generator, and won't be called out specifically in
  future commands.

.. code-block:: console

  ctest --test-dir build

You can run individual tests with the :option:`-R <ctest -R>` flag.

.. code-block:: console

  ctest --test-dir build -R sqrt

Solution
--------

First we add a new executable for the tests.

.. raw:: html

  <details><summary>TODO 1-2: Click to show/hide answer</summary>

.. literalinclude:: Step9/Tests/CMakeLists.txt
  :caption: TODO 1-2: Tests/CMakeLists.txt
  :name: Tests/CMakeLists.txt-add_executable
  :language: cmake
  :start-at: add_executable
  :end-at: TestMathFunctions.cxx
  :append: )

.. raw:: html

  </details>

Then we link in the library we are testing.

.. raw:: html

  <details><summary>TODO 3: Click to show/hide answer</summary>

.. literalinclude:: Step9/Tests/CMakeLists.txt
  :caption: TODO 3: Tests/CMakeLists.txt
  :name: Tests/CMakeLists.txt-target_link_libraries
  :language: cmake
  :start-at: target_link_libraries(TestMathFunctions
  :end-at: )

.. raw:: html

  </details>

We need to call :command:`add_test` for each of the valid operations, but this
would get repetitive, so we write a :command:`function` to do it for us.

.. raw:: html

  <details><summary>TODO 4: Click to show/hide answer</summary>

.. literalinclude:: Step9/Tests/CMakeLists.txt
  :caption: TODO 4: Tests/CMakeLists.txt
  :name: Tests/CMakeLists.txt-function
  :language: cmake
  :start-at: function
  :end-at: endfunction

.. raw:: html

  </details>

Now we can use our :command:`function` to add all the tests.

.. raw:: html

  <details><summary>TODO 5: Click to show/hide answer</summary>

.. literalinclude:: Step9/Tests/CMakeLists.txt
  :caption: TODO 5: Tests/CMakeLists.txt
  :name: Tests/CMakeLists.txt-add_test
  :language: cmake
  :start-at: MathFunctionTest(add
  :end-at: MathFunctionTest(sub

.. raw:: html

  </details>

Finally, we can add the :variable:`BUILD_TESTING` option and conditionally
enable building and running tests in the top-level CML.

.. raw:: html

  <details><summary>TODO 6-7: Click to show/hide answer</summary>

.. literalinclude:: Step9/CMakeLists.txt
  :caption: TODO 6: CMakeLists.txt
  :name: CMakeLists.txt-BUILD_TESTING
  :language: cmake
  :start-at: option(BUILD_TESTING
  :end-at: option(BUILD_TESTING

.. literalinclude:: Step9/CMakeLists.txt
  :caption: TODO 7: CMakeLists.txt
  :name: CMakeLists.txt-enable_testing
  :language: cmake
  :start-at: if(BUILD_TESTING)
  :end-at: endif()

.. raw:: html

  </details>
