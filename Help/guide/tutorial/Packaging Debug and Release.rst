Step 12: Packaging Debug and Release
====================================

**Note:** This example is valid for single-configuration generators and will
not work for multi-configuration generators (e.g. Visual Studio).

By default, CMake's model is that a build directory only contains a single
configuration, be it Debug, Release, MinSizeRel, or RelWithDebInfo. It is
possible, however, to setup CPack to bundle multiple build directories and
construct a package that contains multiple configurations of the same project.

First, we want to ensure that the debug and release builds use different names
for the executables and libraries that will be installed. Let's use `d` as the
postfix for the debug executable and libraries.

Set :variable:`CMAKE_DEBUG_POSTFIX` near the beginning of the top-level
``CMakeLists.txt`` file:

.. literalinclude:: Complete/CMakeLists.txt
  :caption: CMakeLists.txt
  :name: CMakeLists.txt-CMAKE_DEBUG_POSTFIX-variable
  :language: cmake
  :start-after: project(Tutorial VERSION 1.0)
  :end-before: target_compile_features(tutorial_compiler_flags

And the :prop_tgt:`DEBUG_POSTFIX` property on the tutorial executable:

.. literalinclude:: Complete/CMakeLists.txt
  :caption: CMakeLists.txt
  :name: CMakeLists.txt-DEBUG_POSTFIX-property
  :language: cmake
  :start-after: # add the executable
  :end-before: # add the binary tree to the search path for include files

Let's also add version numbering to the ``MathFunctions`` library. In
``MathFunctions/CMakeLists.txt``, set the :prop_tgt:`VERSION` and
:prop_tgt:`SOVERSION` properties:

.. literalinclude:: Complete/MathFunctions/CMakeLists.txt
  :caption: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-VERSION-properties
  :language: cmake
  :start-after: # setup the version numbering
  :end-before: # install rules

From the ``Step12`` directory, create ``debug`` and ``release``
subbdirectories. The layout will look like:

.. code-block:: none

  - Step12
     - debug
     - release

Now we need to setup debug and release builds. We can use
:variable:`CMAKE_BUILD_TYPE` to set the configuration type:

.. code-block:: console

  cd debug
  cmake -DCMAKE_BUILD_TYPE=Debug ..
  cmake --build .
  cd ../release
  cmake -DCMAKE_BUILD_TYPE=Release ..
  cmake --build .

Now that both the debug and release builds are complete, we can use a custom
configuration file to package both builds into a single release. In the
``Step12`` directory, create a file called ``MultiCPackConfig.cmake``. In this
file, first include the default configuration file that was created by the
:manual:`cmake  <cmake(1)>` executable.

Next, use the ``CPACK_INSTALL_CMAKE_PROJECTS`` variable to specify which
projects to install. In this case, we want to install both debug and release.

.. literalinclude:: Complete/MultiCPackConfig.cmake
  :caption: MultiCPackConfig.cmake
  :name: MultiCPackConfig.cmake
  :language: cmake

From the ``Step12`` directory, run :manual:`cpack <cpack(1)>` specifying our
custom configuration file with the ``config`` option:

.. code-block:: console

  cpack --config MultiCPackConfig.cmake
