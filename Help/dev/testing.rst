CMake Testing Guide
*******************

The following is a guide to the CMake test suite for developers.
See documentation on `CMake Development`_ for more information.

See `CMake Integration Testing`_ for running integration testing builds.

See `Tests/README.rst`_ for the test suite layout in the source tree.

.. _`CMake Development`: README.rst
.. _`CMake Integration Testing`: integration-testing.rst
.. _`Tests/README.rst`: ../../Tests/README.rst

Running Tests in the Build Tree
===============================

After `Building CMake`_, one may run the test suite in the build tree
using `ctest(1)`_:

* With a single-configuration CMake generator, such as ``Ninja``
  or ``Unix Makefiles``, one may simply run ``ctest``:

  .. code-block:: console

    $ ctest

* With a multi-configuration CMake generator, such as
  ``Ninja Multi-Config``, ``Visual Studio``, or ``Xcode``,
  one must tell ``ctest`` which configuration to test
  by passing the ``-C <config>`` option:

  .. code-block:: console

    $ ctest -C Debug

Some useful `ctest(1)`_ options include:

``-N``
  List test names without running them.

``-V``
  Show verbose output from each test.

``-j <N>``
  Run to run up to ``N`` tests concurrently.

``-R <regex>``
  Select tests for which the regular expression matches a substring
  of their name.

Cleaning Test Build Trees
-------------------------

Many CMake tests create their own test project build trees underneath
the ``Tests/`` directory at the top of the CMake build tree.  These
build trees are left behind after testing completes in order to
facilitate manual investigation of results.  Many of the tests do *not*
clean their build trees if they are run again, with the exception of
tests using the `RunCMake`_ infrastructure.

In order to clear test build trees, drive the ``test_clean`` custom target
in the CMake build tree:

.. code-block:: console

  $ cmake --build . --target test_clean

This removes the ``Tests/`` subdirectories created by individual tests
so they will use a fresh directory next time they run.

.. _`Building CMake`: ../../README.rst#building-cmake
.. _`ctest(1)`: https://cmake.org/cmake/help/latest/manual/ctest.1.html
.. _`RunCMake`: ../../Tests/RunCMake/README.rst

Running Tests with a Different Generator
========================================

After `Building CMake`_ with one CMake generator, one may configure the
test suite using a different generator in a separate build tree, without
building CMake itself again, by defining ``CMake_TEST_EXTERNAL_CMAKE``
to be the absolute path to the ``bin`` directory containing the ``cmake``,
``ctest``, and ``cpack`` executables.

For example, after building CMake with the ``Ninja`` generator:

.. code-block:: console

  $ cmake -B build-ninja -G Ninja -DCMAKE_BUILD_TYPE=Debug
  $ cmake --build build-ninja

one may configure a second build tree to drive tests with the
``Ninja Multi-Config`` generator:

.. code-block:: console

  $ cmake -B build-nmc-tests -G "Ninja Multi-Config" \
    -DCMake_TEST_EXTERNAL_CMAKE="$PWD/build-ninja/bin"
  $ cmake --build build-nmc-tests --config Release

The second build tree does not build CMake itself, but does configure
the test suite and build test binaries.  One may then run tests normally:

.. code-block:: console

  $ cd build-nmc-tests
  $ ctest -C Release

Note that the configuration with which one drives tests in the second
build tree is independent of the configuration with which CMake was
built in the first.
