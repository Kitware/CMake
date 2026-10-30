.. _`CTest Dashboard Step 1`:

Step 1: A Basic Dashboard Script
================================

This step shows the smallest complete :option:`ctest -S` script that
configures, builds, and tests a project, and submits the results to CDash.

The Example Project
^^^^^^^^^^^^^^^^^^^

Most steps build the project in the ``Project`` directory, which defines
two tests:

``passes``
  A test that exits successfully.

``fails``
  A test that exits with a non-zero exit code.

.. literalinclude:: Project/CMakeLists.txt
  :caption: Project/CMakeLists.txt
  :language: cmake

The project's ``CTestConfig.cmake`` file tells CTest where to submit
results.  These examples submit to the ``sandbox`` project on
`my.cdash.org <https://my.cdash.org/projects>`__:

.. literalinclude:: Project/CTestConfig.cmake
  :caption: Project/CTestConfig.cmake
  :language: cmake

The Script
^^^^^^^^^^

.. literalinclude:: Step1/submit.cmake
  :caption: Step1/submit.cmake
  :language: cmake

The script runs these steps:

:command:`ctest_start`
  Begin a new dashboard submission.

:command:`ctest_update`
  Record the current Git commit and any modified files.

:command:`ctest_configure`
  Run CMake to configure the project.

:command:`ctest_build`
  Build the project.

:command:`ctest_test`
  Run the tests.

:command:`ctest_submit`
  Submit the results to CDash.

Key Concepts
^^^^^^^^^^^^

:variable:`CTEST_BUILD_NAME`
  The name of the build on the dashboard.  Use it to distinguish several
  builds of the same project, such as by platform, compiler, or
  configuration.

:variable:`CTEST_NOTES_FILES`
  Attaches files to the build as notes.  Attaching the script itself makes
  it easy to see what produced a given result.

:variable:`CTEST_UPDATE_VERSION_ONLY`
  Records the current Git commit without updating the source tree.  This is
  useful when a CI system has already checked out the source.

:command:`ctest_empty_binary_directory`
  Removes the build tree before the dashboard starts, so that each run is a
  clean build.

Running the Script
^^^^^^^^^^^^^^^^^^

.. code-block:: shell

  ctest -S Step1/submit.cmake -V

The :option:`ctest -V` option prints the output of each step.
