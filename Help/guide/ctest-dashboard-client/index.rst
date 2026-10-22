CTest Dashboard Client Guide
****************************

.. only:: html

  .. contents::

Introduction
============

A `CDash`_ dashboard collects the results of configuring, building, and
testing a project on many machines.  The :manual:`ctest(1)` program acts
as a :ref:`Dashboard Client`: it runs those steps and submits the results
to a CDash server.

This guide shows how to prepare a project for dashboard submissions, how
to submit results from an existing build tree, and how to write a script
that runs a dashboard on a schedule.  See the :manual:`ctest(1)` manual
for a complete reference.

.. _`CDash`: https://www.cdash.org

Preparing a Project
===================

A project enables dashboard submissions by including the :module:`CTest`
module in its top-level ``CMakeLists.txt`` file:

.. code-block:: cmake

  cmake_minimum_required(VERSION 4.0)
  project(MyProject)

  include(CTest)

  if(BUILD_TESTING)
    add_executable(my_test my_test.cxx)
    add_test(NAME my_test COMMAND my_test)
  endif()

The module calls :command:`enable_testing` and adds the
:variable:`BUILD_TESTING` option.  It also writes the dashboard settings
to a configuration file in the build tree.

The project also needs a ``CTestConfig.cmake`` file next to its top-level
``CMakeLists.txt`` file to say where to submit results.  A CDash server
offers this file for download on the settings page of each project.
It contains settings such as:

.. code-block:: cmake

  set(CTEST_NIGHTLY_START_TIME "01:00:00 UTC")
  set(CTEST_SUBMIT_URL "https://my.cdash.org/submit.php?project=MyProject")

The :variable:`CTEST_NIGHTLY_START_TIME` variable must match the
nightly start time configured in CDash, so that all ``Nightly``
submissions from one night appear on the same day.

Submitting from a Build Tree
============================

The quickest way to submit results is to run :program:`ctest` from an
existing build tree.  Configure and build the project as usual, then run
an ``Experimental`` dashboard:

.. code-block:: shell

  cmake -S MyProject -B build
  cmake --build build
  cd build
  ctest -D Experimental

This runs the ``Start``, ``Configure``, ``Build``, ``Test``,
``Coverage``, and ``Submit`` steps.  The results are recorded in the
``Testing`` directory of the build tree before they are submitted.

The :module:`CTest` module also adds an ``Experimental`` target to the
build system that does the same thing:

.. code-block:: shell

  cmake --build build --target Experimental

Running Individual Steps
------------------------

A dashboard may also be run one step at a time.  The :option:`ctest -M`
option selects the mode and each :option:`ctest -T` option adds a step:

.. code-block:: shell

  ctest -M Experimental -T Start -T Configure -T Build -T Test -T Submit

Results are recorded under the tag created by the most recent ``Start``
step, and later invocations add to it.  By default, the ``Submit`` step
sends every result recorded under that tag, including results from
earlier invocations.  Set :variable:`CTEST_SUBMIT_PARTS` to submit only
some of them.  For example, to rerun the tests and submit only the new
test results:

.. code-block:: shell

  ctest -M Experimental -T Test -T Submit -D CTEST_SUBMIT_PARTS=Test

Overriding Settings
-------------------

Settings in the configuration file can be overridden for one run with the
:option:`-D \<var\>=\<value\> <ctest-dashboard -D>` option.  For example,
to submit with a different build name:

.. code-block:: shell

  ctest -D Experimental -D CTEST_BUILD_NAME=linux-gcc-debug

Writing a Dashboard Script
==========================

Machines that submit dashboards on a schedule usually run a script with
:option:`ctest -S` instead.  A script can check out the source tree,
start from an empty build tree, and choose which steps to run.  It does
not depend on a build tree that was configured beforehand.

The script is written in the :manual:`cmake-language(7)`.  It sets
variables that hold the dashboard settings and calls the
:ref:`CTest Commands` to run each step.  The following script runs a
``Nightly`` dashboard for a project hosted in a Git repository:

.. code-block:: cmake

  # Describe this machine and build.
  set(CTEST_SITE "my-machine")
  set(CTEST_BUILD_NAME "linux-gcc-release")

  # Choose where to put the source and build trees.
  set(CTEST_DASHBOARD_ROOT "$ENV{HOME}/dashboards")
  set(CTEST_SOURCE_DIRECTORY "${CTEST_DASHBOARD_ROOT}/MyProject")
  set(CTEST_BINARY_DIRECTORY "${CTEST_DASHBOARD_ROOT}/MyProject-build")

  # Choose how to configure and build.
  set(CTEST_CMAKE_GENERATOR "Ninja")
  set(CTEST_CONFIGURATION_TYPE "Release")

  # Clone the repository on the first run, and update it later.
  find_program(CTEST_GIT_COMMAND NAMES git)
  set(repo "https://example.com/MyProject.git")
  if(NOT EXISTS "${CTEST_SOURCE_DIRECTORY}")
    set(CTEST_CHECKOUT_COMMAND
      "\"${CTEST_GIT_COMMAND}\" clone ${repo} \"${CTEST_SOURCE_DIRECTORY}\"")
  endif()

  # Start each night with an empty build tree.
  ctest_empty_binary_directory("${CTEST_BINARY_DIRECTORY}")

  ctest_start(Nightly)
  ctest_update()
  ctest_configure()
  ctest_build()
  ctest_test()
  ctest_submit()

Run the script from any directory outside the build tree:

.. code-block:: shell

  ctest -S nightly.cmake -V

The :option:`ctest -V` option shows the progress of each step.

Notes on the script:

* The :command:`ctest_start` command runs the
  :variable:`CTEST_CHECKOUT_COMMAND`, if set, before it reads
  ``CTestConfig.cmake`` from the binary or source tree.
  The script does not need to set :variable:`CTEST_SUBMIT_URL` itself.

* The :command:`ctest_update` command uses :variable:`CTEST_GIT_COMMAND`
  to update the source tree, and records the changes in the submission.

* The :command:`ctest_configure` command runs :manual:`cmake(1)` with the
  generator given by :variable:`CTEST_CMAKE_GENERATOR`.  To pass more
  options to CMake, set :variable:`CTEST_CONFIGURE_COMMAND` or use the
  ``OPTIONS`` argument of the command.

* The :command:`ctest_empty_binary_directory` command does nothing if the
  build tree does not exist yet.  To avoid removing an unrelated directory
  by mistake, it fails if the directory exists but does not contain a
  ``CMakeCache.txt`` file.

Passing Values to a Script
--------------------------

One script can serve several builds by reading values from the command
line.  Each :option:`-D \<var\>=\<value\> <ctest-dashboard -D>` option sets
a variable before the script runs:

.. code-block:: cmake

  if(NOT DEFINED CTEST_BUILD_NAME)
    set(CTEST_BUILD_NAME "linux-gcc-release")
  endif()

.. code-block:: shell

  ctest -S nightly.cmake -D CTEST_BUILD_NAME=linux-gcc-debug

Variables whose names do not begin with ``CTEST_`` must be given with a
type, such as ``-D dashboard_model:STRING=Nightly``.

Continuous Dashboards
---------------------

A ``Continuous`` dashboard runs repeatedly during the day and submits
results only when the source tree has changed.  In a script, use the
``RETURN_VALUE`` option of :command:`ctest_update` to stop early when
nothing changed:

.. code-block:: cmake

  ctest_start(Continuous)
  ctest_update(RETURN_VALUE count)
  if(count EQUAL 0)
    return()
  endif()
  ctest_configure()
  ctest_build()
  ctest_test()
  ctest_submit()

A ``Continuous`` script usually keeps its build tree between runs, so it
does not call :command:`ctest_empty_binary_directory`.  This keeps each
build incremental.

Checking for Failures
---------------------

Each command can store its result in a variable so the script can react
to failures.  For example, to skip the tests when the build fails:

.. code-block:: cmake

  ctest_build(NUMBER_ERRORS errors)
  if(errors EQUAL 0)
    ctest_test()
  endif()
  ctest_submit()

See the documentation of each command for the values it reports.

Running on a Schedule
=====================

A dashboard script can be run by the system scheduler (such as ``crontab`` on
Linux and macOS), or via a continuous integration platform (such as GitHub
Actions).  Schedule ``Nightly`` scripts to start shortly after the
:variable:`CTEST_NIGHTLY_START_TIME` so that each submission is associated with
the intended day.

Using CMake Presets
===================

.. versionadded:: 4.4

A project that provides :manual:`cmake-presets(7)` can use them in a
dashboard.  In a script, set :variable:`CTEST_PRESET` to use one preset
name for the configure, build, and test steps, or set
:variable:`CTEST_CONFIGURE_PRESET`, :variable:`CTEST_BUILD_PRESET`, and
:variable:`CTEST_TEST_PRESET` separately:

.. code-block:: cmake

  set(CTEST_SOURCE_DIRECTORY "${CTEST_DASHBOARD_ROOT}/MyProject")
  set(CTEST_BINARY_DIRECTORY "${CTEST_DASHBOARD_ROOT}/MyProject-build")
  set(CTEST_PRESET "ci-release")

  ctest_start(Nightly)
  ctest_configure()
  ctest_build()
  ctest_test()
  ctest_submit()

Settings from the script, such as :variable:`CTEST_BINARY_DIRECTORY`,
take priority over the corresponding settings from the preset.

From the command line, the :ctest-dashboard-option:`--source-dir` option
lets the ``Configure`` step create a new build tree from a preset.  If
the ``ci-release`` preset sets ``binaryDir`` to
``${sourceDir}/build/${presetName}``, then:

.. code-block:: shell

  ctest -M Experimental -T Configure --source-dir MyProject \
    -D CTEST_CONFIGURE_PRESET=ci-release
  ctest -M Experimental -T Build -T Test -T Submit \
    --test-dir MyProject/build/ci-release

Adding More Steps
=================

Coverage
--------

To record test coverage, build the project with coverage enabled, such as
with the ``--coverage`` flag of GCC, and call :command:`ctest_coverage`
after :command:`ctest_test`:

.. code-block:: cmake

  find_program(CTEST_COVERAGE_COMMAND NAMES gcov)

  ctest_start(Nightly)
  ctest_configure(OPTIONS "-DCMAKE_CXX_FLAGS=--coverage")
  ctest_build()
  ctest_test()
  ctest_coverage()
  ctest_submit()

See the :ref:`CTest Coverage Step` for the supported tools.

Memory Checking
---------------

To run the tests under a memory checker, call :command:`ctest_memcheck`
instead of, or in addition to, :command:`ctest_test`.  The
:variable:`CTEST_MEMORYCHECK_TYPE` variable selects the tool:

.. code-block:: cmake

  set(CTEST_MEMORYCHECK_TYPE "Valgrind")
  find_program(CTEST_MEMORYCHECK_COMMAND NAMES valgrind)

  ctest_start(Nightly)
  ctest_configure()
  ctest_build()
  ctest_memcheck()
  ctest_submit()

See the :ref:`CTest MemCheck Step` for the supported tools, including
the sanitizers supported by Clang and GCC.

Detailed Build Reports
----------------------

By default, CTest finds build errors and warnings by scanning the build
output.  Set :variable:`CTEST_USE_LAUNCHERS` in ``CTestConfig.cmake`` to
instead report the command line and output of each compile or link step
that produces a warning or error:

.. code-block:: cmake

  set(CTEST_USE_LAUNCHERS 1)

This works with the :ref:`Makefile Generators` and :ref:`Ninja Generators`.

Notes and Extra Files
---------------------

A submission can include notes files, such as a copy of the dashboard
script, to show on CDash.  Set :variable:`CTEST_NOTES_FILES` before
calling :command:`ctest_submit`:

.. code-block:: cmake

  set(CTEST_NOTES_FILES "${CMAKE_CURRENT_LIST_FILE}")

From the command line, use the :ctest-dashboard-option:`--add-notes` option.

Next Steps
==========

* The :ref:`Dashboard Client` section of the :manual:`ctest(1)` manual
  documents the command-line options, steps, and modes.

* The :ref:`Dashboard Client Configuration` sections document every setting for
  each step.

* The :ref:`CTest Commands` documentation describes the options of each
  command used in a dashboard script.
