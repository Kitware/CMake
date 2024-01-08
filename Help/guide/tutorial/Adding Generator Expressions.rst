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
empty string, and ``$<1:...>`` results in the content of ``...``.  They can also
be nested.

Exercise 1 - Adding Compiler Warning Flags with Generator Expressions
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

Open the file ``Step4/CMakeLists.txt`` and complete ``TODO 1`` through
``TODO 4``.

First, in the top level ``CMakeLists.txt`` file, we need to set the
:command:`cmake_minimum_required` to ``3.15``. In this exercise we are going
to use a generator expression which was introduced in CMake 3.15.

Next we add the desired compiler warning flags that we want for our project.
As warning flags vary based on the compiler, we use the
``COMPILE_LANG_AND_ID`` generator expression to control which flags to apply
given a language and a set of compiler ids.

Build and Run
-------------

Make a new directory called ``Step4_build``, run the :manual:`cmake <cmake(1)>`
executable or the :manual:`cmake-gui <cmake-gui(1)>` to configure the project
and then build it with your chosen build tool or by using ``cmake --build .``
from the build directory.

.. code-block:: console

  mkdir Step4_build
  cd Step4_build
  cmake ../Step4
  cmake --build .

Solution
--------

Update the :command:`cmake_minimum_required` to require at least CMake
version ``3.15``:

.. raw:: html

  <details><summary>TODO 1: Click to show/hide answer</summary>

.. literalinclude:: Step5/CMakeLists.txt
  :caption: TODO 1: CMakeLists.txt
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

  <details><summary>TODO 2: Click to show/hide answer</summary>

.. literalinclude:: Step5/CMakeLists.txt
  :caption: TODO 2: CMakeLists.txt
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

  <details><summary>TODO 3: Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 3: CMakeLists.txt
  :name: CMakeLists.txt-compile_flags

  target_compile_options(tutorial_compiler_flags INTERFACE
    "$<${gcc_like_cxx}:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>"
    "$<${msvc_cxx}:-W3>"
  )

.. raw:: html

  </details>

Lastly, we only want these warning flags to be used during builds. Consumers
of our installed project should not inherit our warning flags. To specify
this, we wrap our flags from TODO 3 in a generator expression using the
``BUILD_INTERFACE`` condition. The resulting full code looks like the following:

.. raw:: html

  <details><summary>TODO 4: Click to show/hide answer</summary>

.. literalinclude:: Step5/CMakeLists.txt
  :caption: TODO 4: CMakeLists.txt
  :name: CMakeLists.txt-target_compile_options-genex
  :language: cmake
  :start-after: set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
  :end-before: # configure a header file to pass some of the CMake settings

.. raw:: html

  </details>
