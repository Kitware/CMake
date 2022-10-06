Step 6: Adding Support for a Testing Dashboard
==============================================

Adding support for submitting our test results to a dashboard is simple. We
already defined a number of tests for our project in
:ref:`Testing Support <Tutorial Testing Support>`. Now we just have to run
those tests and submit them to CDash.


Exercise 1 - Send Results to a Testing Dashboard
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Goal
----

Display our CTest results with CDash.

Helpful Resources
-----------------

* :manual:`ctest(1)`
* :command:`include`
* :module:`CTest`

Files to Edit
-------------

* ``CMakeLists.txt``

Getting Started
---------------

For this exercise, complete ``TODO 1`` in the top-level ``CMakeLists.txt`` by
including the :module:`CTest` module. This will enable testing with CTest as
well as dashboard submissions to CDash, so we can safely remove the call to
:command:`enable_testing`.

We will also need to acquire a ``CTestConfig.cmake`` file to be placed in the
top-level directory. When run, the :manual:`ctest <ctest(1)>` executable will
read this file to gather information about the testing dashboard. It contains:

* The project name

* The project "Nightly" start time

  *  The time when a 24 hour "day" starts for this project.

* The URL of the CDash instance where the submission's generated documents
  will be sent

For this tutorial, a public dashboard server is used and its corresponding
``CTestConfig.cmake`` file is provided for you in this step's root directory.
In practice, this file would be downloaded from a project's ``Settings`` page
on the CDash instance intended to host the test results. Once downloaded from
CDash, the file should not be modified locally.

.. literalinclude:: Step7/CTestConfig.cmake
  :caption: CTestConfig.cmake
  :name: CTestConfig.cmake
  :language: cmake


Build and Run
-------------

Note that as part of the CDash submission some information about your
development system (e.g. site name or full pathnames) may displayed publicly.

To create a simple test dashboard, run the :manual:`cmake <cmake(1)>`
executable or the :manual:`cmake-gui <cmake-gui(1)>` to configure the project
but do not build it yet. Instead, navigate to the build directory and run:

.. code-block:: console

  ctest [-VV] -D Experimental

Remember, for multi-config generators (e.g. Visual Studio), the configuration
type must be specified:

.. code-block:: console

  ctest [-VV] -C Debug -D Experimental

Or, from an IDE, build the ``Experimental`` target.

The :manual:`ctest <ctest(1)>` executable will build the project, run any
tests, and submit the results to Kitware's public dashboard:
https://my.cdash.org/index.php?project=CMakeTutorial.

Solution
--------

The only CMake code changed needed in this step was to enable dashboard
submissions to CDash by including the :module:`CTest` module in our top-level
``CMakeLists.txt``:

.. raw:: html

  <details><summary>TODO 1: Click to show/hide answer</summary>

.. literalinclude:: Step7/CMakeLists.txt
  :caption: TODO 1: CMakeLists.txt
  :name: CMakeLists.txt-include-CTest
  :language: cmake
  :start-after: # enable testing
  :end-before: # does the application run

.. raw:: html

  </details>
