Step 4: In-Depth CMake Target Commands
======================================

There are several target commands within CMake we can use to describe
requirements. As a reminder, a target command is one which modifies the
properties of the target it is applied to. These properties describe
requirements needed to build the software, such as sources, compile flags,
and output names; or properties necessary to consume the target, such as header
includes, library directories, and linkage rules.

.. note::
  As discussed in ``Step1``, properties required to build a target should be
  described with the ``PRIVATE`` :ref:`scope keyword <Target Command Scope>`,
  those required to consume the target with ``INTERFACE``, and properties needed
  for both are described with ``PUBLIC``.

In this step we will go over all the available target commands in CMake. Not all
target commands are created equal. We have already discussed the two most
important target commands, :command:`target_sources` and
:command:`target_link_libraries`. Of the remaining commands, some are almost
as common as these two, others have more advanced applications, and a couple
should only be used as a last resort when other options are not available.

Background
^^^^^^^^^^

Before going any further, let's name all of the CMake target commands. We'll
split these into three groups: the recommended and generally useful commands,
the advanced and cautionary commands, and the "footgun" commands which should
be avoided unless necessary.

+-----------------------------------------+--------------------------------------+---------------------------------------+
| Common/Recommended                      | Advanced/Caution                     | Esoteric/Footguns                     |
+=========================================+======================================+=======================================+
| :command:`target_compile_definitions`   | :command:`get_target_property`       | :command:`target_include_directories` |
| :command:`target_compile_features`      | :command:`set_target_properties`     | :command:`target_link_directories`    |
| :command:`target_link_libraries`        | :command:`target_compile_options`    |                                       |
| :command:`target_sources`               | :command:`target_link_options`       |                                       |
|                                         | :command:`target_precompile_headers` |                                       |
+-----------------------------------------+--------------------------------------+---------------------------------------+

.. note::
    There's no such thing as a "bad" CMake target command. They all have valid
    use cases. This categorization is provided to give newcomers a simple
    intuition about which commands they should consider first when tackling
    a problem.

We'll demonstrate most of these in the following exercises. The three we won't
be using are :command:`get_target_property`, :command:`set_target_properties`
and :command:`target_precompile_headers`, so we will briefly discuss their
purpose here.

The :command:`get_target_property` and :command:`set_target_properties` commands
give direct access to a target's properties by name. They can even be used
to attach arbitrary property names to a target.

.. code-block:: cmake

  add_library(Example)
  set_target_properties(Example
    PROPERTIES
      Key Value
      Hello World
  )

  get_target_property(KeyVar Example Key)
  get_target_property(HelloVar Example Hello)

  message("Key: ${KeyVar}")
  message("Hello: ${HelloVar}")

.. code-block:: console

  $ cmake -B build
  ...
  Key: Value
  Hello: World

The full list of target properties which are semantically meaningful to CMake
are documented at :manual:`cmake-properties(7)`, however most of these should
be modified with their dedicated commands. For example, it is unnecessary to
directly manipulate ``LINK_LIBRARIES`` and ``INTERFACE_LINK_LIBRARIES``, as
these are handled by :command:`target_link_libraries`.

Conversely, some lesser-used properties are only accessible via these commands.
The :prop_tgt:`DEPRECATION` property, used to attach deprecation notices to
targets, can only be set via :command:`set_target_properties`; as can the
:prop_tgt:`ADDITIONAL_CLEAN_FILES`, for describing additional files to be
removed by CMake's ``clean`` target; and other properties of this sort.

The :command:`target_precompile_headers` command takes a list of header files,
similar to :command:`target_sources`, and creates a precompiled header from
them. This precompiled header is then force included into all translation
units in the target. This can be useful for build performance.

Exercise 1 - Features and Definitions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In earlier steps we cautioned against globally setting
:variable:`CMAKE_<LANG>_STANDARD` and overriding packagers' decision concerning
which language standard to use. On the other hand, many libraries have a
minimum required feature set they need in order to build, and for these it
is appropriate to use the :command:`target_compile_features` command to
communicate those requirements.

.. code-block:: cmake

  target_compile_features(MyApp PRIVATE cxx_std_20)

The :command:`target_compile_features` command describes a minimum language
standard as a target property. If the :variable:`CMAKE_<LANG>_STANDARD` is above
this version, or the compiler default already provides this language standard,
no action is taken. If additional flags are necessary to enable the standard,
these will be added by CMake.

.. note::
  :command:`target_compile_features` manipulates the same style of interface and
  non-interface properties as the other target commands. This means it is
  possible to *inherit* a language standard requirement specified with
  ``INTERFACE`` or ``PUBLIC`` scope keywords.

  If language features are used only in implementation files, then the
  respective compile features should be ``PRIVATE``. If the target's headers
  use the features, then ``PUBLIC`` or ``INTERFACE`` should be used.

For C++, the compile features are of the form ``cxx_std_YY`` where ``YY`` is
the standardization year, e.g. ``14``, ``17``, ``20``, etc.

The :command:`target_compile_definitions` command describes compile definitions
as target properties. It is the most common mechanism for communicating build
configuration information to the source code itself. As with all properties,
the scope keywords apply as we have discussed.

.. code-block:: cmake

  target_compile_definitions(MyLibrary
    PRIVATE
      MYLIBRARY_USE_EXPERIMENTAL_IMPLEMENTATION

    PUBLIC
      MYLIBRARY_EXCLUDE_DEPRECATED_FUNCTIONS
  )

It is neither required nor desired that we attach ``-D`` prefixes to compile
definitions described with :command:`target_compile_definitions`. CMake will
determine the correct flag for the current compiler.

Goal
----

Use :command:`target_compile_features` and :command:`target_compile_definitions`
to communicate language standard and compile definition requirements.

Helpful Resources
-----------------

* :command:`target_compile_features`
* :command:`target_compile_definitions`
* :command:`option`
* :command:`if`

Files to Edit
-------------

* ``Tutorial/CMakeLists.txt``
* ``MathFunctions/CMakeLists.txt``
* ``MathFunctions/MathFunctions.cxx``
* ``CMakePresets.json``

Getting Started
---------------

The ``Help/guide/tutorial/Step4`` directory contains the complete, recommended
solution to ``Step3`` and relevant ``TODOs`` for this step. Complete ``TODO 1``
through ``TODO 8``.

Build and Run
-------------

We can run CMake using our ``tutorial`` preset, and then build as usual.

.. code-block:: console

  cmake --preset tutorial
  cmake --build build

Verify that the output of ``Tutorial`` is what we would expect for ``std::sqrt``.

Solution
--------

First we add a new option to the top-level CML.

.. raw:: html

  <details><summary>TODO 1: Click to show/hide answer</summary>

.. literalinclude:: Step5/CMakeLists.txt
  :caption: TODO 1: CMakeLists.txt
  :name: CMakeLists.txt-TUTORIAL_USE_STD_SQRT
  :language: cmake
  :start-at: option(TUTORIAL_BUILD_UTILITIES
  :end-at: option(TUTORIAL_USE_STD_SQRT

.. raw:: html

  </details>

Then we add the compile feature and definitions to ``MathFunctions``.

.. raw:: html

  <details><summary>TODO 2-3: Click to show/hide answer</summary>

.. literalinclude:: Step5/MathFunctions/CMakeLists.txt
  :caption: TODO 2-3: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-target_compile_features
  :language: cmake
  :start-at: target_compile_features
  :end-at: endif()

.. raw:: html

  </details>

And the compile feature for ``Tutorial``.

.. raw:: html

  <details><summary>TODO 4: Click to show/hide answer</summary>

.. literalinclude:: Step5/Tutorial/CMakeLists.txt
  :caption: TODO 4: Tutorial/CMakeLists.txt
  :name: Tutorial/CMakeLists.txt-target_compile_features
  :language: cmake
  :start-at: target_compile_features
  :end-at: target_compile_features

.. raw:: html

  </details>

Now we can modify ``MathFunctions`` to take advantage of the new definition.

.. raw:: html

  <details><summary>TODO 5-6: Click to show/hide answer</summary>

.. literalinclude:: Step5/MathFunctions/MathFunctions.cxx
  :caption: TODO 5: MathFunctions/MathFunctions.cxx
  :name: MathFunctions/MathFunctions.cxx-cmath
  :language: c++
  :start-at: cmath
  :end-at: format
  :append: #include <iostream>

.. literalinclude:: Step5/MathFunctions/MathFunctions.cxx
  :caption: TODO 6: MathFunctions/MathFunctions.cxx
  :name: MathFunctions/MathFunctions.cxx-std-sqrt
  :language: c++
  :start-at: double sqrt(double x)
  :end-at: }

.. raw:: html

  </details>

Finally we can update our ``CMakePresets.json``. We don't need to set
``CMAKE_CXX_STANDARD`` anymore, but we do want to try out our new
compile definition.

.. raw:: html

  <details><summary>TODO 7-8: Click to show/hide answer</summary>

.. code-block:: json
  :caption: TODO 7-8: CMakePresets.json
  :name: CMakePresets.json-std-sqrt

  "cacheVariables": {
    "TUTORIAL_USE_STD_SQRT": "ON"
  }

.. raw:: html

  </details>

Exercise 2 - Compile and Link Options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Sometimes, we need to exercise specific control over the exact options being
passed on the compile and link line. These situations are addressed by
:command:`target_compile_options` and :command:`target_link_options`.

.. code:: cmake

  target_compile_options(MyApp PRIVATE -Wall -Werror)
  target_link_options(MyApp PRIVATE -T LinksScript.ld)

There are several problems with unconditionally calling
:command:`target_compile_options` or :command:`target_link_options`. The primary
problem is compiler flags are specific to the compiler frontend being used. In
order to ensure that our project supports multiple compiler frontends, we must
only pass compatible flags to the compiler.

We can achieve this by checking the :variable:`CMAKE_<LANG>_COMPILER_FRONTEND_VARIANT`
variable which tells us the style of flags supported by the compiler frontend.

.. note::
  Prior to CMake 3.26, :variable:`CMAKE_<LANG>_COMPILER_FRONTEND_VARIANT` was
  only set for compilers with multiple frontend variants. In versions after
  CMake 3.26 checking this variable alone is sufficient.

  However this tutorial targets CMake 3.23. As such, the logic is more
  complicated than we have time for here. This tutorial step already includes
  correct logic for checking the compiler variant for MSVC, GCC, Clang, and
  AppleClang on CMake 3.23.

Even if a compiler accepts the flags we pass, the semantics of compiler flags
change over time. This is especially true with regards to warnings. Projects
should not turn warnings-as-error flags by default, as this can break their
build on otherwise innocuous compiler warnings included in later releases.

.. note::
  For errors and warnings, consider placing flags in :variable:`CMAKE_<LANG>_FLAGS`
  for local development builds and during CI runs (via preset or
  :option:`-D <cmake -D>` flags). We know exactly which compiler and
  toolchain are being used in these contexts, so we can customize the behavior
  precisely without risking build breakages on other platforms.

Goal
----

Add appropriate warning flags to the ``Tutorial`` executable for MSVC-style and
GNU-style compiler frontends.

Helpful Resources
-----------------

* :command:`target_compile_options`

Files to Edit
-------------

* ``Tutorial/CMakeLists.txt``

Getting Started
---------------

Continue editing files in the ``Step4`` directory. The conditional for checking
the frontend variant has already been written. Complete ``TODO 9`` and
``TODO 10`` to add warning flags to ``Tutorial``.

Build and Run
-------------

Since we have already configured for this step, we can build with the usual
command.

.. code-block:: cmake

  cmake --build build

This should reveal a simple warning in the build. You can go ahead and fix it.

Solution
--------

We need to add two compile options to ``Tutorial``, one MSVC-style flag and
one GNU-style flag.

.. raw:: html

  <details><summary>TODO 9-10: Click to show/hide answer</summary>

.. literalinclude:: Step5/Tutorial/CMakeLists.txt
  :caption: TODO 9-10: Tutorial/CMakeLists.txt
  :name: Tutorial/CMakeLists.txt-target_compile_options
  :language: cmake
  :start-at: if(
  :end-at: endif()

.. raw:: html

  </details>

Exercise 3 - Include and Link Directories
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. note::
  This exercise requires building an archive using a compiler directly on the
  command line. It is not used in later steps. It is included only to
  demonstrate a use case for :command:`target_include_directories` and
  :command:`target_link_directories`.

  If you cannot complete this exercise for whatever reason feel free to treat
  it as informational-only, or skip it entirely.

It is generally unnecessary to directly describe include and link directories,
as these requirements are inherited when linking together targets generated
within CMake, or from external dependencies imported into CMake with commands
we will cover in later steps.

If we happen to have some libraries or header files which are not described
by a CMake target which we need to bring into the build, perhaps pre-compiled
binaries provided by a vendor, we can incorporate with the
:command:`target_link_directories` and :command:`target_include_directories`
commands.

.. code-block:: cmake

  target_link_directories(MyApp PRIVATE Vendor/lib)
  target_include_directories(MyApp PRIVATE Vendor/include)


These commands use properties which map to the ``-L`` and ``-I`` compiler flags
(or whatever flags the compiler uses for link and include directories).

Of course, passing a link directory doesn't tell the compiler to link anything
into the build. For that we need :command:`target_link_libraries`. When
:command:`target_link_libraries` is given an argument which does not map to
a target name, it will add the string directly to the link line as a library
to be linked into the build (prepending any appropriate flags, such a ``-l``).

Goal
----

Describe a pre-compiled, vendored, static library and its headers inside a
project using :command:`target_link_directories` and
:command:`target_include_directories`.

Helpful Resources
-----------------

* :command:`target_link_directories`
* :command:`target_include_directories`
* :command:`target_link_libraries`

Files to Edit
-------------

* ``Vendor/CMakeLists.txt``
* ``Tutorial/CMakeLists.txt``

Getting Started
---------------

You will need to build the vendor library into a static archive to complete this
exercise. Navigate to the ``Help/guide/tutorial/Step4/Vendor/lib`` directory
and build the code as appropriate for your platform.

Typical commands for a GCC toolchain on Unix-like systems are:

.. code-block:: console

  g++ -c Vendor.cxx
  ar rvs libVendor.a Vendor.o

Likewise, sample commands for an MSVC toolchain on Windows are:

.. code-block:: console

  cl -c Vendor.cxx
  lib -out:Vendor.lib Vendor.obj

Here, since you're directly invoking ``cl`` and ``lib``, make sure to use a
Developer Command Prompt for your version of Visual Studio with the same
target architecture used by this CMake project.

Then complete ``TODO 11`` through ``TODO 14``.

.. note::
  ``VendorLib`` is an ``INTERFACE`` library, meaning it has no build requirements
  (because it has already been built). All of its properties should also be
  interface properties.

  We'll discuss ``INTERFACE`` libraries in greater depth during the next step.


Build and Run
-------------

If you have successfully built ``libVendor``, you can rebuild ``Tutorial``
using the normal command.

.. code-block:: console

  cmake --build build

Running ``Tutorial`` should now output a message about the acceptability of the
result to the vendor.

Solution
--------

We need to use the target link and include commands to describe the archive
and its headers as ``INTERFACE`` requirements of ``VendorLib``.

.. raw:: html

  <details><summary>TODO 11-13: Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 11-13: Vendor/CMakeLists.txt
  :name: Vendor/CMakeLists.txt

  target_include_directories(VendorLib
    INTERFACE
      include
  )

  target_link_directories(VendorLib
    INTERFACE
      lib
  )

  target_link_libraries(VendorLib
    INTERFACE
      Vendor
  )

.. raw:: html

  </details>

Then we can add ``VendorLib`` to ``Tutorial``'s linked libraries.

.. raw:: html

  <details><summary>TODO 14: Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 14: Tutorial/CMakeLists.txt
  :name: Tutorial/CMakeLists.txt-VendorLib

  target_link_libraries(Tutorial
    PRIVATE
      MathFunctions
      VendorLib
  )

.. raw:: html

  </details>
