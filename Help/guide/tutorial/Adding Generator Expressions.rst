Step 4: Adding Generator Expressions
=====================================

:manual:`Generator expressions <cmake-generator-expressions(7)>` are evaluated
during build system generation to produce information specific to each build
configuration.

:manual:`Generator expressions <cmake-generator-expressions(7)>` are allowed in
the context of many target properties, such as :prop_tgt:`LINK_LIBRARIES`,
:prop_tgt:`INCLUDE_DIRECTORIES`, :prop_tgt:`COMPILE_DEFINITIONS` and others.
They may also be used when using commands to populate those properties, such as
:command:`target_link_libraries`, :command:`target_include_directories`,
:command:`target_compile_definitions` and others.

:manual:`Generator expressions <cmake-generator-expressions(7)>`  may be used
to enable conditional linking, conditional definitions used when compiling,
conditional include directories and more. The conditions may be based on the
build configuration, target properties, platform information or any other
queryable information.

There are different types of
:manual:`generator expressions <cmake-generator-expressions(7)>` including
Logical, Informational, and Output expressions.

Logical expressions are used to create conditional output. The basic
expressions are the ``0`` and ``1`` expressions. A ``$<0:...>`` results in the
empty string, and ``<1:...>`` results in the content of ``...``.  They can also
be nested.

Exercise 1 - Setting the C++ Standard with Interface Libraries
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Before we use :manual:`generator expressions <cmake-generator-expressions(7)>`
let's refactor our existing code to use an ``INTERFACE`` library. We will
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

The starting source code is provided in the ``Step4`` directory. In this
exercise, complete ``TODO 1`` through ``TODO 3``.

Start by editing the top level ``CMakeLists.txt`` file. Construct an
``INTERFACE`` library target called ``tutorial_compiler_flags`` and
specify ``cxx_std_11`` as a target compiler feature.

Modify ``CMakeLists.txt`` and ``MathFunctions/CMakeLists.txt`` so that all
targets have a :command:`target_link_libraries` call to
``tutorial_compiler_flags``.

Build and Run
-------------

Make a new directory called ``Step4_build``, run the :manual:`cmake <cmake(1)>`
executable or the :manual:`cmake-gui <cmake-gui(1)>` to configure the project
and then build it with your chosen build tool or by using ``cmake --build .``
from the build directory.

Here's a refresher of what that looks like from the command line:

.. code-block:: console

  mkdir Step4_build
  cd Step4_build
  cmake ../Step4
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

.. literalinclude:: Step4/CMakeLists.txt
  :caption: CMakeLists.txt
  :name: CMakeLists.txt-CXX_STANDARD-variable-remove
  :language: cmake
  :start-after: # specify the C++ standard
  :end-before: # TODO 5: Create helper variables

Next, we need to create an interface library, ``tutorial_compiler_flags``. And
then use :command:`target_compile_features` to add the compiler feature
``cxx_std_11``.


.. raw:: html

  <details><summary>TODO 1: Click to show/hide answer</summary>

.. literalinclude:: Step5/CMakeLists.txt
  :caption: TODO 1: CMakeLists.txt
  :name: CMakeLists.txt-cxx_std-feature
  :language: cmake
  :start-after: # specify the C++ standard
  :end-before: # add compiler warning flags just

.. raw:: html

  </details>

Finally, with our interface library set up, we need to link our
executable ``Target`` and our ``MathFunctions`` library to our new
``tutorial_compiler_flags`` library. Respectively, the code will look like
this:

.. raw:: html

  <details><summary>TODO 2: Click to show/hide answer</summary>

.. literalinclude:: Step5/CMakeLists.txt
  :caption: TODO 2: CMakeLists.txt
  :name: CMakeLists.txt-target_link_libraries-step4
  :language: cmake
  :start-after: add_executable(Tutorial tutorial.cxx)
  :end-before: # add the binary tree to the search path for include file

.. raw:: html

  </details>

and this:

.. raw:: html

  <details><summary>TODO 3: Click to show/hide answer</summary>

.. literalinclude:: Step5/MathFunctions/CMakeLists.txt
  :caption: TODO 3: MathFunctions/CMakeLists.txt
  :name: MathFunctions-CMakeLists.txt-target_link_libraries-step4
  :language: cmake
  :start-after: # link our compiler flags interface library
  :end-before: # TODO 1

.. raw:: html

  </details>

With this, all of our code still requires C++ 11 to build. Notice
though that with this method, it gives us the ability to be specific about
which targets get specific requirements. In addition, we create a single
source of truth in our interface library.

Exercise 2 - Adding Compiler Warning Flags with Generator Expressions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A common usage of
:manual:`generator expressions <cmake-generator-expressions(7)>` is to
conditionally add compiler flags, such as those for language levels or
warnings. A nice pattern is to associate this information to an ``INTERFACE``
target allowing this information to propagate.

Goal
----

Add compiler warning flags when building but not for installed versions.

Helpful Resources
-----------------

* :manual:`cmake-generator-expressions(7)`
* :command:`cmake_minimum_required`
* :command:`set`
* :command:`target_compile_options`

Files to Edit
-------------

* ``CMakeLists.txt``

Getting Started
---------------

Start with the resulting files from Exercise 1. Complete ``TODO 4`` through
``TODO 7``.

First, in the top level ``CMakeLists.txt`` file, we need to set the
:command:`cmake_minimum_required` to ``3.15``. In this exercise we are going
to use a generator expression which was introduced in CMake 3.15.

Next we add the desired compiler warning flags that we want for our project.
As warning flags vary based on the compiler, we use the
``COMPILE_LANG_AND_ID`` generator expression to control which flags to apply
given a language and a set of compiler ids.

Build and Run
-------------

Since we have our build directory already configured from Exercise 1, simply
rebuild our code by calling the following:

.. code-block:: console

  cd Step4_build
  cmake --build .

Solution
--------

Update the :command:`cmake_minimum_required` to require at least CMake
version ``3.15``:

.. raw:: html

  <details><summary>TODO 4: Click to show/hide answer</summary>

.. literalinclude:: Step5/CMakeLists.txt
  :caption: TODO 4: CMakeLists.txt
  :name: MathFunctions-CMakeLists.txt-minimum-required-step4
  :language: cmake
  :end-before: # set the project name and version

.. raw:: html

  </details>

Next we determine which compiler our system is currently using to build
since warning flags vary based on the compiler we use. This is done with
the ``COMPILE_LANG_AND_ID`` generator expression. We set the result in the
variables ``gcc_like_cxx`` and ``msvc_cxx`` as follows:

.. raw:: html

  <details><summary>TODO 5: Click to show/hide answer</summary>

.. literalinclude:: Step5/CMakeLists.txt
  :caption: TODO 5: CMakeLists.txt
  :name: CMakeLists.txt-compile_lang_and_id
  :language: cmake
  :start-after: # the BUILD_INTERFACE genex
  :end-before: target_compile_options(tutorial_compiler_flags INTERFACE

.. raw:: html

  </details>

Next we add the desired compiler warning flags that we want for our project.
Using our variables ``gcc_like_cxx`` and ``msvc_cxx``, we can use another
generator expression to apply the respective flags only when the variables are
true. We use :command:`target_compile_options` to apply these flags to our
interface library.

.. raw:: html

  <details><summary>TODO 6: Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 6: CMakeLists.txt
  :name: CMakeLists.txt-compile_flags

  target_compile_options(tutorial_compiler_flags INTERFACE
    "$<${gcc_like_cxx}:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>"
    "$<${msvc_cxx}:-W3>"
  )

.. raw:: html

  </details>

Lastly, we only want these warning flags to be used during builds. Consumers
of our installed project should not inherit our warning flags. To specify
this, we wrap our flags in a generator expression using the ``BUILD_INTERFACE``
condition. The resulting full code looks like the following:

.. raw:: html

  <details><summary>TODO 7: Click to show/hide answer</summary>

.. literalinclude:: Step5/CMakeLists.txt
  :caption: TODO 7: CMakeLists.txt
  :name: CMakeLists.txt-target_compile_options-genex
  :language: cmake
  :start-after: set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
  :end-before: # should we use our own math functions

.. raw:: html

  </details>
