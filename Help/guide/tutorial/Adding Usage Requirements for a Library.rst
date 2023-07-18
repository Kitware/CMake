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
  :end-before: # should we use our own

.. raw:: html

  </details>

Now that we've specified usage requirements for ``MathFunctions`` we can
safely remove our uses of the ``EXTRA_INCLUDES`` variable from the top-level
``CMakeLists.txt``.

Remove this line:

.. raw:: html

  <details><summary>TODO 2: Click to show/hide answer</summary>

.. literalinclude:: Step3/CMakeLists.txt
  :caption: TODO 2: CMakeLists.txt
  :name: CMakeLists.txt-remove-EXTRA_INCLUDES
  :language: cmake
  :start-after: add_subdirectory(MathFunctions)
  :end-before: # add the executable

.. raw:: html

  </details>

And remove ``EXTRA_INCLUDES`` from ``target_include_directories``:

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

Exercise 2 - Setting the C++ Standard with Interface Libraries
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Now that we have switched our code to a more modern approach, let's demonstrate
a modern technique to set properties to multiple targets.

Let's refactor our existing code to use an ``INTERFACE`` library. We will
use that library in the next step to demonstrate a common use for
:manual:`generator expressions <cmake-generator-expressions(7)>`.

Goal
----

Add an ``INTERFACE`` library target to specify the required C++ standard.

Helpful Resources
-----------------

* :command:`add_library`
* :command:`target_compile_features`
* :command:`target_link_libraries`

Files to Edit
-------------

* ``CMakeLists.txt``
* ``MathFunctions/CMakeLists.txt``

Getting Started
---------------

In this exercise, we will refactor our code to use an ``INTERFACE`` library to
specify the C++ standard.

Start this exercise from what we left at the end of Step3 exercise 1. You will
have to complete ``TODO 4`` through ``TODO 7``.

Start by editing the top level ``CMakeLists.txt`` file. Construct an
``INTERFACE`` library target called ``tutorial_compiler_flags`` and
specify ``cxx_std_11`` as a target compiler feature.

Modify ``CMakeLists.txt`` and ``MathFunctions/CMakeLists.txt`` so that all
targets have a :command:`target_link_libraries` call to
``tutorial_compiler_flags``.

Build and Run
-------------

Since we have our build directory already configured from Exercise 1, simply
rebuild our code by calling the following:

.. code-block:: console

  cd Step3_build
  cmake --build .

Next, use the newly built ``Tutorial`` and verify that it is working as
expected.

Solution
--------

Let's update our code from the previous step to use interface libraries
to set our C++ requirements.

To start, we need to remove the two :command:`set` calls on the variables
:variable:`CMAKE_CXX_STANDARD` and :variable:`CMAKE_CXX_STANDARD_REQUIRED`.
The specific lines to remove are as follows:

.. literalinclude:: Step3/CMakeLists.txt
  :caption: CMakeLists.txt
  :name: CMakeLists.txt-CXX_STANDARD-variable-remove
  :language: cmake
  :start-after: # specify the C++ standard
  :end-before: # configure a header file

Next, we need to create an interface library, ``tutorial_compiler_flags``. And
then use :command:`target_compile_features` to add the compiler feature
``cxx_std_11``.


.. raw:: html

  <details><summary>TODO 4: Click to show/hide answer</summary>

.. literalinclude:: Step4/CMakeLists.txt
  :caption: TODO 4: CMakeLists.txt
  :name: CMakeLists.txt-cxx_std-feature
  :language: cmake
  :start-after: # specify the C++ standard
  :end-before: # TODO 2: Create helper

.. raw:: html

  </details>

Finally, with our interface library set up, we need to link our
executable ``Target``, our ``MathFunctions`` library, and our ``SqrtLibrary``
library to our new
``tutorial_compiler_flags`` library. Respectively, the code will look like
this:

.. raw:: html

  <details><summary>TODO 5: Click to show/hide answer</summary>

.. literalinclude:: Step4/CMakeLists.txt
  :caption: TODO 5: CMakeLists.txt
  :name: CMakeLists.txt-target_link_libraries-step4
  :language: cmake
  :start-after: add_executable(Tutorial tutorial.cxx)
  :end-before: # add the binary tree to the search path for include file

.. raw:: html

  </details>

this:

.. raw:: html

  <details><summary>TODO 6: Click to show/hide answer</summary>

.. literalinclude:: Step4/MathFunctions/CMakeLists.txt
  :caption: TODO 6: MathFunctions/CMakeLists.txt
  :name: MathFunctions-CMakeLists.txt-target_link_libraries-step4
  :language: cmake
  :start-after: # link our compiler flags interface library
  :end-before: target_link_libraries(MathFunctions

.. raw:: html

  </details>

and this:

.. raw:: html

  <details><summary>TODO 7: Click to show/hide answer</summary>

.. literalinclude:: Step4/MathFunctions/CMakeLists.txt
  :caption: TODO 7: MathFunctions/CMakeLists.txt
  :name: MathFunctions-SqrtLibrary-target_link_libraries-step4
  :language: cmake
  :start-after: # link our compiler flags interface library
  :end-before: target_link_libraries(MathFunctions PUBLIC SqrtLibrary)

.. raw:: html

  </details>


With this, all of our code still requires C++ 11 to build. Notice
though that with this method, it gives us the ability to be specific about
which targets get specific requirements. In addition, we create a single
source of truth in our interface library.
