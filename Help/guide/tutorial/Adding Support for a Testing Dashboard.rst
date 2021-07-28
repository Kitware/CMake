Step 8: Adding Support for a Testing Dashboard
==============================================

Adding support for submitting our test results to a dashboard is simple. We
already defined a number of tests for our project in
:ref:`Testing Support <Tutorial Testing Support>`. Now we just have to run
those tests and submit them to a dashboard. To include support for dashboards
we include the :module:`CTest` module in our top-level ``CMakeLists.txt``.

Replace:

.. code-block:: cmake
  :caption: CMakeLists.txt
  :name: CMakeLists.txt-enable_testing-remove

  # enable testing
  enable_testing()

With:

.. code-block:: cmake
  :caption: CMakeLists.txt
  :name: CMakeLists.txt-include-CTest

  # enable dashboard scripting
  include(CTest)

The :module:`CTest` module will automatically call ``enable_testing()``, so we
can remove it from our CMake files.

We will also need to create a ``CTestConfig.cmake`` file in the top-level
directory where we can specify the name of the project and where to submit the
dashboard.

.. literalinclude:: Step9/CTestConfig.cmake
  :caption: CTestConfig.cmake
  :name: CTestConfig.cmake
  :language: cmake

The :manual:`ctest <ctest(1)>` executable will read in this file when it runs.
To create a simple dashboard you can run the :manual:`cmake <cmake(1)>`
executable or the :manual:`cmake-gui <cmake-gui(1)>` to configure the project,
but do not build it yet. Instead, change directory to the binary tree, and then
run:

.. code-block:: console

  ctest [-VV] -D Experimental

Remember, for multi-config generators (e.g. Visual Studio), the configuration
type must be specified:

.. code-block:: console

  ctest [-VV] -C Debug -D Experimental

Or, from an IDE, build the ``Experimental`` target.

The :manual:`ctest <ctest(1)>` executable will build and test the project and
submit the results to Kitware's public dashboard:
https://my.cdash.org/index.php?project=CMakeTutorial.
