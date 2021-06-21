Step 3: Adding Usage Requirements for a Library
===============================================

Usage requirements allow for far better control over a library or executable's
link and include line while also giving more control over the transitive
property of targets inside CMake. The primary commands that leverage usage
requirements are:

  - :command:`target_compile_definitions`
  - :command:`target_compile_options`
  - :command:`target_include_directories`
  - :command:`target_link_libraries`

Let's refactor our code from :guide:`tutorial/Adding a Library` to use the
modern CMake approach of usage requirements. We first state that anybody
linking to ``MathFunctions`` needs to include the current source directory,
while ``MathFunctions`` itself doesn't. So this can become an ``INTERFACE``
usage requirement.

Remember ``INTERFACE`` means things that consumers require but the producer
doesn't. Add the following lines to the end of
``MathFunctions/CMakeLists.txt``:

.. literalinclude:: Step4/MathFunctions/CMakeLists.txt
  :caption: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-target_include_directories-INTERFACE
  :language: cmake
  :start-after: # to find MathFunctions.h

Now that we've specified usage requirements for ``MathFunctions`` we can safely
remove our uses of the ``EXTRA_INCLUDES`` variable from the top-level
``CMakeLists.txt``, here:

.. literalinclude:: Step4/CMakeLists.txt
  :caption: CMakeLists.txt
  :name: CMakeLists.txt-remove-EXTRA_INCLUDES
  :language: cmake
  :start-after: # add the MathFunctions library
  :end-before: # add the executable

And here:

.. literalinclude:: Step4/CMakeLists.txt
  :caption: CMakeLists.txt
  :name: CMakeLists.txt-target_include_directories-remove-EXTRA_INCLUDES
  :language: cmake
  :start-after: # so that we will find TutorialConfig.h

Once this is done, run the :manual:`cmake  <cmake(1)>` executable or the
:manual:`cmake-gui <cmake-gui(1)>` to configure the project and then build it
with your chosen build tool or by using ``cmake --build .`` from the build
directory.
