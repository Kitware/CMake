Step 3: Adding Usage Requirements for a Library
===============================================

Exercise 1 - Adding Usage Requirements for a Library
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:ref:`Usage requirements <Target Usage Requirements>` of a target parameters
allow for far better control over a library or executable's link and include
line while also giving more control over the transitive property of targets
inside CMake. The primary commands that
leverage usage requirements are:

* :command:`target_compile_definitions`
* :command:`target_compile_options`
* :command:`target_include_directories`
* :command:`target_link_directories`
* :command:`target_link_options`
* :command:`target_precompile_headers`
* :command:`target_sources`


Goal
----

Add usage requirements for a library.

Helpful Materials
-----------------

* :variable:`CMAKE_CURRENT_SOURCE_DIR`

Files to Edit
-------------

* ``MathFunctions/CMakeLists.txt``
* ``CMakeLists.txt``

Getting Started
---------------

In this exercise, we will refactor our code from
:guide:`tutorial/Adding a Library` to use the modern CMake approach. We will
let our library define its own usage requirements so they are passed
transitively to other targets as necessary. In this case, ``MathFunctions``
will specify any needed include directories itself. Then, the consuming target
``Tutorial`` simply needs to link to ``MathFunctions`` and not worry about
any additional include directories.

The starting source code is provided in the ``Step3`` directory. In this
exercise, complete ``TODO 1`` through ``TODO 3``.

First, add a call to :command:`target_include_directories` in
``MathFunctions/CMakeLists``. Remember that
:variable:`CMAKE_CURRENT_SOURCE_DIR` is the path to the source directory
currently being processed.

Then, update (and simplify!) the call to
:command:`target_include_directories` in the top-level ``CMakeLists.txt``.

Build and Run
-------------

Make a new directory called ``Step3_build``, run the :manual:`cmake
<cmake(1)>` executable or the :manual:`cmake-gui <cmake-gui(1)>` to
configure the project and then build it with your chosen build tool or by
using :option:`cmake --build . <cmake --build>` from the build directory.
Here's a refresher of what that looks like from the command line:

.. code-block:: console

  mkdir Step3_build
  cd Step3_build
  cmake ../Step3
  cmake --build .

Next, use the newly built ``Tutorial`` and verify that it is working as
expected.

Solution
--------

Let's update the code from the previous step to use the modern CMake
approach of usage requirements.

We want to state that anybody linking to ``MathFunctions`` needs to include
the current source directory, while ``MathFunctions`` itself doesn't. This
can be expressed with an ``INTERFACE`` usage requirement. Remember
``INTERFACE`` means things that consumers require but the producer doesn't.

At the end of ``MathFunctions/CMakeLists.txt``, use
:command:`target_include_directories` with the ``INTERFACE`` keyword, as
follows:

.. raw:: html

  <details><summary>TODO 1: Click to show/hide answer</summary>

.. literalinclude:: Step4/MathFunctions/CMakeLists.txt
  :caption: TODO 1: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-target_include_directories-INTERFACE
  :language: cmake
  :start-after: # to find MathFunctions.h
  :end-before: # TODO 3: Link to

.. raw:: html

  </details>

Now that we've specified usage requirements for ``MathFunctions`` we can
safely remove our uses of the ``EXTRA_INCLUDES`` variable from the top-level
``CMakeLists.txt``, here:

.. raw:: html

  <details><summary>TODO 2: Click to show/hide answer</summary>

.. literalinclude:: Step4/CMakeLists.txt
  :caption: TODO 2: CMakeLists.txt
  :name: CMakeLists.txt-remove-EXTRA_INCLUDES
  :language: cmake
  :start-after: # add the MathFunctions library
  :end-before: # add the executable

.. raw:: html

  </details>

And here:

.. raw:: html

  <details><summary>TODO 3: Click to show/hide answer</summary>

.. literalinclude:: Step4/CMakeLists.txt
  :caption: TODO 3: CMakeLists.txt
  :name: CMakeLists.txt-target_include_directories-remove-EXTRA_INCLUDES
  :language: cmake
  :start-after: # so that we will find TutorialConfig.h

.. raw:: html

  </details>

Notice that with this technique, the only thing our executable target does to
use our library is call :command:`target_link_libraries` with the name
of the library target. In larger projects, the classic method of specifying
library dependencies manually becomes very complicated very quickly.
