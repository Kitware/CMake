This directory contains tests that run CMake to configure a project
but do not actually build anything.  To add a test:

1. Add a subdirectory named for the test, say ``<Test>/``.

2. In ``./CMakeLists.txt`` call ``add_RunCMake_test`` and pass the
   test directory name ``<Test>``.

3. Create script ``<Test>/RunCMakeTest.cmake`` in the directory containing::

    include(RunCMake)
    run_cmake(SubTest1)
    ...
    run_cmake(SubTestN)

   where ``SubTest1`` through ``SubTestN`` are sub-test names each
   corresponding to an independent CMake run and project configuration.

   One may also add calls of the form::

    run_cmake_command(SubTestI ${CMAKE_COMMAND} ...)

   to fully customize the test case command-line.

   Alternatively, if the test is to cover running ``ctest -S`` then use::

    include(RunCTest)
    run_ctest(SubTest1)
    ...
    run_ctest(SubTestN)

   and create ``test.cmake.in``, ``CTestConfig.cmake.in``, and
   ``CMakeLists.txt.in`` files to be configured for each case.

4. Create file ``<Test>/CMakeLists.txt`` in the directory containing::

    cmake_minimum_required(...)
    project(${RunCMake_TEST} NONE) # or languages needed
    include(${RunCMake_TEST}.cmake)

   where ``${RunCMake_TEST}`` is literal.  A value for ``RunCMake_TEST``
   will be passed to CMake by the ``run_cmake`` macro when running each
   sub-test.

5. Create a ``<Test>/<SubTest>.cmake`` file for each sub-test named
   above containing the actual test code.  Optionally create files
   containing expected test results:

   ``<SubTest>-result.txt``
    Regex matching expected process result, if not ``0``
   ``<SubTest>-stdout.txt``
    Regex matching expected stdout content
   ``<SubTest>-stderr.txt``
    Regex matching expected stderr content, if not ``^$``
   ``<SubTest>-check.cmake``
    Custom result check.

  Note that when a specific platform expects differing stdout or stderr that
  can be done by adding a platform specific output file. These follow the
  naming convention of:
   ``<SubTest>-stdout-<platform_lower_case>.txt``
   ``<SubTest>-stderr-<platform_lower_case>.txt``

   Note that trailing newlines will be stripped from actual and expected
   test output before matching against the stdout and stderr expressions.
   The code in ``<SubTest>-check.cmake`` may use variables

   ``RunCMake_TEST_SOURCE_DIR``
    Top of test source tree
   ``RunCMake_TEST_BINARY_DIR``
    Top of test binary tree

   and an failure must store a message in ``RunCMake_TEST_FAILED``.

To speed up local testing, you can choose to run only a subset of
``run_cmake()`` tests in a ``RunCMakeTest.cmake`` script by using the
``RunCMake_TEST_FILTER`` environment variable. If this variable is set,
it is treated as a regular expression, and any tests whose names don't
match the regular expression are not run. For example::

  $ RunCMake_TEST_FILTER="^example" ctest -R '^RunCMake\.Example$'

This will only run subtests in ``RunCMake.Example`` that start with
``example``.

To speed up the process of creating a new ``RunCMake`` test, you can run a
script that will automatically perform steps 1 through 4 for you::

  cmake -DRunCMake_TEST_SUITE=<test suite name> -P Tests/RunCMake/AddRunCMakeTestSuite.cmake

Be sure to run this from the top-level CMake source directory.
