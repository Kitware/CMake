CMake Tests/RunCMake Directory
******************************

This directory contains tests that run CMake and/or other tools while
precisely checking their return code and stdout/stderr content.
The RunCMake infrastructure is useful for testing error cases and
diagnostic output.

See also `../README.rst`_, the `CMake Testing Guide`_,
and the `CMake Source Code Guide`_.

.. _`../README.rst`: ../README.rst
.. _`CMake Testing Guide`: ../../Help/dev/testing.rst
.. _`CMake Source Code Guide`: ../../Help/dev/source.rst
.. _`CMakeLists.txt`: CMakeLists.txt

Adding a Test
=============

To add a test:

1. Add a subdirectory named for the test, say ``<Test>/``.

2. In `CMakeLists.txt`_ call ``add_RunCMake_test`` and pass the
   test directory name ``<Test>``.

3. Create script ``<Test>/RunCMakeTest.cmake`` in the directory containing::

    include(RunCMake)
    run_cmake(Case1)
    ...
    run_cmake(CaseN)

   where ``Case1`` through ``CaseN`` are case names each corresponding to
   an independent CMake run and project configuration.

   One may also add calls of the form::

    run_cmake_command(CaseI ${CMAKE_COMMAND} ...)

   to fully customize the test case command-line.

   Alternatively, if the test is to cover running ``ctest -S`` then use::

    include(RunCTest)
    run_ctest(Case1)
    ...
    run_ctest(CaseN)

   and create ``test.cmake.in``, ``CTestConfig.cmake.in``, and
   ``CMakeLists.txt.in`` files to be configured for each case.

   Alternatively, if the test is to cover running ``cpack -G`` then use::

    include(RunCPack)
    run_cpack(Sample1)
    ...
    run_cpack(SampleN)

   where ``Sample1`` through ``SampleN`` are sample project directories
   in the ``RunCPack/`` directory adjacent to this file.

4. Create file ``<Test>/CMakeLists.txt`` in the directory containing::

    cmake_minimum_required(...)
    project(${RunCMake_TEST} NONE) # or languages needed
    include(${RunCMake_TEST}.cmake)

   where ``${RunCMake_TEST}`` is literal.  A value for ``RunCMake_TEST``
   will be passed to CMake by the ``run_cmake`` macro when running each
   case.

5. Create a ``<Test>/<case>.cmake`` file for each case named
   above containing the actual test code.  Optionally create files
   containing expected test results:

   ``<case>-result.txt``
    Regex matching expected process result, if not ``0``
   ``<case>-stdout.txt``
    Regex matching expected stdout content
   ``<case>-stderr.txt``
    Regex matching expected stderr content, if not ``^$``
   ``<case>-check.cmake``
    Custom result check.

   To specify platform-specific matches, create files of the form
   ``<case>-{stdout,stderr}-<platform_lower_case>.txt``.

   Note that trailing newlines will be stripped from actual and expected
   test output before matching against the stdout and stderr expressions.
   The code in ``<case>-check.cmake`` may use the `RunCMake Variables`_.
   On failure the script must store a message in ``RunCMake_TEST_FAILED``.
   The check script may optionally set ``RunCMake_TEST_FAILURE_MESSAGE``
   with additional text to be included in the message if the test fails.

RunCMake Commands
=================

A ``RunCMakeTest.cmake`` script, after ``include(RunCMake)``, may use
the following commands.

``run_cmake(<case>)``
  Run CMake or another command and check expected results described by
  ``<case>-{result,stdout,stderr}.txt`` and ``<case>-check.cmake``.
  The command is executed by a call of the form::

    execute_process(
      COMMAND ${RunCMake_TEST_COMMAND} ${RunCMake_TEST_OPTIONS}
      WORKING_DIRECTORY "${RunCMake_TEST_COMMAND_WORKING_DIRECTORY}"
      [TIMEOUT "${RunCMake_TEST_TIMEOUT}"]
      ...
      )

  Behavior may be customized by setting `RunCMake Variables`_ before
  the call.

``run_cmake_command(<case> <command> <args>...)``
  Sets ``RunCMake_TEST_COMMAND`` to ``<command>;<args>...``
  and calls ``run_cmake(<case>)``.

  This is useful to run an arbitrary command.

``run_cmake_script(<case> <args>...)``
  Sets ``RunCMake_TEST_COMMAND`` to
  ``${CMAKE_COMMAND};<args>...;-P;${RunCMake_SOURCE_DIR}/<case>.cmake``
  and calls ``run_cmake(<case>)``.

  This is useful to run CMake in script mode without configuring a project.

``run_cmake_with_options(<case> <opts>...)``
  Sets ``RunCMake_TEST_OPTIONS`` to ``<opts>...``
  and calls ``run_cmake(<case>)``.

``run_cmake_with_raw_args(<case> "<args>")``
  Calls ``run_cmake(<case>)`` with the underlying ``execute_process()``
  call extended with the content of ``<args>`` treated as literal source
  code of CMake language command arguments::

    execute_process(
      COMMAND ${RunCMake_TEST_COMMAND} ${RunCMake_TEST_OPTIONS} <args>
      ...
      )

  This is useful to pass arguments to the test command that cannot be
  encoded in CMake language ``;``-separated lists.

RunCMake Variables
==================

The behavior of `RunCMake Commands`_ such as ``run_cmake()`` may be
customized by setting the following variables before a call.

``RunCMake_GENERATOR``
  CMake generator to use when configuring projects.
  This provided to ``RunCMakeTest.cmake`` scripts automatically
  when they are executed, based on the CMake generator used to
  configure the test suite.

  For some generators, additional variables are also provided:

  ``RunCMake_GENERATOR_PLATFORM``
    Specifies the ``CMAKE_GENERATOR_PLATFORM``.

  ``RunCMake_GENERATOR_TOOLSET``
    Specifies the ``CMAKE_GENERATOR_TOOLSET``.

  ``RunCMake_GENERATOR_INSTANCE``
    Specifies the ``CMAKE_GENERATOR_INSTANCE``.

``RunCMake_GENERATOR_IS_MULTI_CONFIG``
  Boolean value indicating whether ``${RunCMake_GENERATOR}`` is a
  multi-config generator.
  This provided to ``RunCMakeTest.cmake`` scripts automatically
  when they are executed, based on the CMake generator used to
  configure the test suite.

``RunCMake_SOURCE_DIR``
  Absolute path to the ``Tests/RunCMake/<Test>`` directory in
  the CMake source tree.  This provided to ``RunCMakeTest.cmake``
  scripts automatically when they are executed.

``RunCMake_BINARY_DIR``
  Absolute path to the ``Tests/RunCMake/<Test>`` directory in
  the CMake binary tree.  This provided to ``RunCMakeTest.cmake``
  scripts automatically when they are executed.

``RunCMake_TEST_SOURCE_DIR``
  Absolute path to the individual test case's source tree.
  If not set, defaults to ``${RunCMake_SOURCE_DIR}``.

``RunCMake_TEST_BINARY_DIR``
  Absolute path to the individual test case's binary tree.
  If not set, defaults to ``${RunCMake_BINARY_DIR}/<case>-build``.

``RunCMake_TEST_NO_CLEAN``
  Boolean value indicating whether ``run_cmake(<case>)`` should remove the
  ``${RunCMake_TEST_BINARY_DIR}`` directory before running the test case.
  If not set, or if set to a false value, the directory is removed.

  This is useful to run `Multi-Step Test Cases`_.

``RunCMake_TEST_COMMAND``
  The command for ``run_cmake(<case>)`` to execute.
  If not set, defaults to running CMake to generate a project::

    ${CMAKE_COMMAND} ${RunCMake_TEST_SOURCE_DIR} \
      -G ${RunCMake_GENERATOR} ... -DRunCMake_TEST=<case>

``RunCMake_TEST_COMMAND_WORKING_DIRECTORY``
  The working directory in which ``run_cmake(<case>)`` to execute its command.
  If not set, defaults to ``${RunCMake_TEST_BINARY_DIR}``.

``RunCMake_TEST_OPTIONS``
  Additional command-line options for ``run_cmake(<case>)`` to pass to
  CMake when configuring a project with a default ``RunCMake_TEST_COMMAND``.
  If not set, defaults to empty.
  If ``RunCMake_TEST_COMMAND`` is set, ``RunCMake_TEST_OPTIONS`` is forced
  to empty.

``RunCMake_TEST_OUTPUT_MERGE``
  Boolean value indicating whether ``run_cmake(<case>)`` should redirect
  the test process's ``stderr`` into its ``stdout``.

``RunCMake_TEST_TIMEOUT``
  Specify a timeout, in seconds, for ``run_cmake(<case>)`` to pass to its
  underlying ``execute_process()`` call using the ``TIMEOUT`` option.

Multi-Step Test Cases
=====================

Normally each ``run_cmake(<case>)`` call corresponds to one standalone
test case with its own build tree.  However, some test cases may require
multiple steps to be performed in a single build tree.  This can be
achieved as follows::

  block()
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/example-build)
    run_cmake(example)
    set(RunCMake_TEST_NO_CLEAN 1)
    set(RunCMake_TEST_OUTPUT_MERGE 1)
    run_cmake_command(example-build ${CMAKE_COMMAND} --build . --config Debug)
  endblock()

In this example, ``block() ... endblock()`` is used to isolate the
variable settings from later cases.  A single build tree is used for
all cases inside the block.  The first step cleans the build tree and
runs CMake to configure the case's project.  The second step runs
``cmake --build`` to drive the generated build system and merges the
build tool's ``stderr`` into its ``stdout``.  Note that each call uses
a unique case name so that expected results can be expressed individually.

Running a Test
==============

Each call to ``add_RunCMake_test(Example)`` in `CMakeLists.txt`_ creates
a test named ``RunCMake.Example`` that may be run with ``ctest``::

  $ ctest -R "^RunCMake\.Example$"

To speed up local testing, you can choose to run only a subset of
``run_cmake()`` tests in a ``RunCMakeTest.cmake`` script by using the
``RunCMake_TEST_FILTER`` environment variable. If this variable is set,
it is treated as a regular expression, and any tests whose names don't
match the regular expression are not run. For example::

  $ RunCMake_TEST_FILTER="^example" ctest -R '^RunCMake\.Example$'

This will only run cases in ``RunCMake.Example`` that start with
``example``.

To speed up the process of creating a new ``RunCMake`` test, you can run a
script that will automatically perform steps 1 through 4 for you::

  cmake -DRunCMake_TEST_SUITE=<test suite name> -P Tests/RunCMake/AddRunCMakeTestSuite.cmake

Be sure to run this from the top-level CMake source directory.
