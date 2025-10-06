Step 3: Configuration and Cache Variables
=========================================

CMake projects often have some project-specific configuration variables which
users and packagers are interested in. CMake has many ways that an invoking
user or process can communicate these configuration choices, but the most
fundamental of them are :option:`-D <cmake -D>` flags.

In this step we'll explore the ins and out of how to provide project
configuration options from within a CML, and how to invoke CMake to take
advantage of configuration options provided by both CMake and individual
projects.

Background
^^^^^^^^^^

If we had a CMake project for compression software which supported multiple
compression algorithms, we might want to let the packager of the project decide
which algorithms to enable when they build our software. We can do so by
consuming variables set via :option:`-D <cmake -D>` flags.

.. code-block:: cmake

  if(COMPRESSION_SOFTWARE_USE_ZLIB)
    message("I will use Zlib!")
    # ...
  endif()

  if(COMPRESSION_SOFTWARE_USE_ZSTD)
    message("I will use Zstd!")
    # ...
  endif()

.. code-block:: console

  $ cmake -B build \
      -DCOMPRESSION_SOFTWARE_USE_ZLIB=ON \
      -DCOMPRESSION_SOFTWARE_USE_ZSTD=OFF
  ...
  I will use Zlib!

Of course, we will want to provide reasonable defaults for these configuration
choices, and a way to communicate the purpose of a given option. This function
is provided by the :command:`option` command.

.. code-block:: cmake

  option(COMPRESSION_SOFTWARE_USE_ZLIB "Support Zlib compression" ON)
  option(COMPRESSION_SOFTWARE_USE_ZSTD "Support Zstd compression" ON)

  if(COMPRESSION_SOFTWARE_USE_ZLIB)
    # Same as before
  # ...

.. code-block:: console

  $ cmake -B build \
      -DCOMPRESSION_SOFTWARE_USE_ZLIB=OFF
  ...
  I will use Zstd!

The names created by :option:`-D <cmake -D>` flags and :command:`option` are
not normal variables, they are **cache** variables. Cache variables are globally
visible variables which are *sticky*, their value is difficult to change after
it is initially set. In fact they are so sticky that, in project mode, CMake
will save and restore cache variables across multiple configurations. If a
cache variable is set once, it will remain until another :option:`-D <cmake -D>`
flag preempts the saved variable.

.. note::
  CMake itself has dozens of normal and cache variables used for configuration.
  These are documented at :manual:`cmake-variables(7)` and operate in the same
  manner as project-provided variables for configuration.

:command:`set` can also be used to manipulate cache variables, but will not
change a variable which has already been created.

.. code-block:: cmake

  set(StickyCacheVariable "I will not change" CACHE STRING "")
  set(StickyCacheVariable "Overwrite StickyCache" CACHE STRING "")

  message("StickyCacheVariable: ${StickyCacheVariable}")

.. code-block:: console

  $ cmake -P StickyCacheVariable.cmake
  StickyCacheVariable: I will not change

Because :option:`-D <cmake -D>` flags are processed before any other commands,
they take precedence for setting the value of a cache variable.

.. code-block:: console

  $ cmake \
    -DStickyCacheVariable="Commandline always wins" \
    -P StickyCacheVariable.cmake
  StickyCacheVariable: Commandline always wins

While cache variables cannot ordinarily be changed, they can be *shadowed* by
normal variables. We can observe this by :command:`set`'ing a variable to have
the same name as a cache variable, and then using :command:`unset` to remove
the normal variable.

.. code-block:: cmake

  set(ShadowVariable "In the shadows" CACHE STRING "")
  set(ShadowVariable "Hiding the cache variable")
  message("ShadowVariable: ${ShadowVariable}")

  unset(ShadowVariable)
  message("ShadowVariable: ${ShadowVariable}")

.. code-block:: console

  $ cmake -P ShadowVariable.cmake
  ShadowVariable: Hiding the cache variable
  ShadowVariable: In the shadows

Exercise 1 - Using Options
^^^^^^^^^^^^^^^^^^^^^^^^^^

We can imagine a scenario where consumers really want our ``MathFunctions``
library, and the ``Tutorial`` utility is a "take it or leave it" add-on. In
that case, we might want to add an option to allow consumers to disable
building our ``Tutorial`` binary, building only the ``MathFunctions`` library.

With our knowledge of options, conditionals, and cache variables we have all
the pieces we need to make this configuration available.

Goal
----

Add an option named ``TUTORIAL_BUILD_UTILITIES`` to control if the ``Tutorial``
binary is configured and built.

.. note::
  CMake allows us to determine which targets are built after configuration. Our
  users could ask for the ``MathFunctions`` library alone without ``Tutorial``.
  CMake also has mechanisms to exclude targets from ``ALL``, the default target
  which builds all the other available targets.

  However, options which completely exclude targets from the configuration are
  convenient and popular, especially if configuring those targets involves
  heavy-weight steps which might take some time.

  It also simplifies :command:`install()` logic, which we'll discuss in later
  steps, if targets the packager is uninterested in are completely excluded.

Helpful Resources
-----------------

* :command:`option`
* :command:`if`

Files to Edit
-------------

* ``CMakeLists.txt``

Getting Started
---------------

The ``Help/guide/tutorial/Step3`` folder contains the complete, recommended
solution to ``Step1`` and the relevant ``TODOs`` for this step. Take a minute
to review and refamiliarize yourself with the ``Tutorial`` project.

When you feel you have an understanding of the current code, start with
``TODO 1`` and complete through ``TODO 2``.

Build and Run
-------------

We can now reconfigure our project. However, this time we want to control the
configuration via :option:`-D <cmake -D>` flags. We again start by navigating
to ``Help/guide/tutorial/Step3`` and invoking CMake, but this time with our
configuration options.

.. code-block:: console

  cmake -B build -DTUTORIAL_BUILD_UTILITIES=OFF

We can now build as usual.

.. code-block:: console

  cmake --build build

After the build we should observe no Tutorial executable is produced. Because
cache variables are sticky even a reconfigure shouldn't change this, despite
the default-``ON`` option.

.. code-block:: console

  cmake -B build
  cmake --build build

Will not produce the Tutorial executable, the cache variables are "locked in".
To change this we have two options. First, we can edit the file which stores
the cache variables between CMake configuration runs, the "CMake Cache". This
file is ``build/CMakeCache.txt``, in it we can find the option cache variable.

.. code-block:: text

  //Build the Tutorial executable
  TUTORIAL_BUILD_UTILITIES:BOOL=OFF

We can change this from ``OFF`` to ``ON``, rerun the build, and we will get
our ``Tutorial`` executable.

.. note::
  ``CMakeCache.txt`` entries are of the form ``<Name>:<Type>=<Value>``, however
  the "type" is only a hint. All objects in CMake are strings, regardless of
  what the cache says.

Alternatively, we can change the value of the cache variable on the command
line, because the command line runs before ``CMakeCache.txt`` is loaded its
value take precedence over those in the cache file.

.. code-block:: console

  cmake -B build -DTUTORIAL_BUILD_UTILITIES=ON
  cmake --build build

Doing so we observe the value in ``CMakeCache.txt`` has flipped from ``OFF``
to ``ON``, and that the ``Tutorial`` executable is built.

Solution
--------

First we create our :command:`option` to provide our cache variable with a
reasonable default value.

.. raw:: html

  <details><summary>TODO 1: Click to show/hide answer</summary>

.. literalinclude:: Step4/CMakeLists.txt
  :caption: TODO 1: CMakeLists.txt
  :name: CMakeLists.txt-option-TUTORIAL_BUILD_UTILITIES
  :language: cmake
  :start-at: option(TUTORIAL_BUILD_UTILITIES
  :end-at: option(TUTORIAL_BUILD_UTILITIES

.. raw:: html

  </details>

Then we can check the cache variable to conditionally enable the ``Tutorial``
executable (by way of adding its subdirectory).

.. raw:: html

  <details><summary>TODO 2: Click to show/hide answer</summary>

.. literalinclude:: Step4/CMakeLists.txt
  :caption: TODO 2: CMakeLists.txt
  :name: CMakeLists.txt-if-TUTORIAL_BUILD_UTILITIES
  :language: cmake
  :start-at: if(TUTORIAL_BUILD_UTILITIES)
  :end-at: endif()

.. raw:: html

  </details>

Exercise 2 - ``CMAKE`` Variables
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

CMake has several important normal and cache variables provided to allow
packagers to control the build. Decisions such as compilers, default flags,
search locations for packages, and much more are all controlled by CMake's
own configuration variables.

Among the most important are language standards. As the language standard can
have significant impact on the ABI presented by a given package. For example,
it's quite common for libraries to use standard C++ templates on later
standards, and provide polyfills on earlier standards. If a library is consumed
under different standards then ABI incompatibilities between the standard
templates and the polyfills can result in incomprehensible errors and runtime
crashes.

Ensuring all of our targets are built under the same language standard is
achieved with the :variable:`CMAKE_<LANG>_STANDARD` cache variables. For C++,
this is ``CMAKE_CXX_STANDARD``.

.. note::
  Because these variables are so important, it is equally important that
  developers not override or shadow them in their CMLs. Shadowing
  :variable:`CMAKE_<LANG>_STANDARD` in a CML because the library wants C++20,
  when the packager has decided to build the rest of their libraries and
  applications with C++23, can lead to the aforementioned terrible,
  incomprehensible errors.

  Do not :command:`set` ``CMAKE_`` globals without very strong reasons for
  doing so. We'll discuss better methods for targets to communicate
  requirements like definitions and minimum standards in later steps.

In this exercise, we'll introduce some C++20 code into our library and
executable and build them with C++20 by setting the appropriate cache variable.

Goal
----

Use ``std::format`` to format printed strings instead of stream operators. To
ensure availability of ``std::format``, configure CMake to use the C++20
standard for C++ targets.

Helpful Resources
-----------------

* :option:`cmake -D`
* :variable:`CMAKE_<LANG>_STANDARD`
* :variable:`CMAKE_CXX_STANDARD`
* :prop_tgt:`CXX_STANDARD`
* `cppreference \<format\> <https://en.cppreference.com/w/cpp/utility/format/format.html>`_

Files to Edit
-------------

* ``Tutorial/Tutorial.cxx``
* ``MathFunctions/MathFunctions.cxx``

Getting Started
---------------

Continue to edit files from ``Step3``. Complete ``TODO 3`` through ``TODO 7``.
We'll be modifying our prints to use ``std::format`` instead of stream
operators.

Ensure your cache variables are set such that the Tutorial executable will be
built, using any of the methods discussed in the previous exercise.

Build and Run
-------------

We need to reconfigure our project with the new standard, we can do this
using the same method as our ``TUTORIAL_BUILD_UTILITIES`` cache variable.

.. code-block:: console

  cmake -B build -DCMAKE_CXX_STANDARD=20

.. note::
  Configuration variables are, by convention, prefixed with the provider of the
  variable. CMake configuration variables are prefixed with ``CMAKE_``, while
  projects should prefix their variables with ``<PROJECT>_``.

  The tutorial configuration variables follow this convention, and are prefixed
  with ``TUTORIAL_``.

Now that we've configured with C++20, we can build as usual.

.. code-block:: console

  cmake --build build

Solution
--------

We need to include ``<format>`` and then use it.

.. raw:: html

  <details><summary>TODO 3-5: Click to show/hide answer</summary>

.. literalinclude:: Step4/Tutorial/Tutorial.cxx
  :caption: TODO 3: Tutorial/Tutorial.cxx
  :name: Tutorial/Tutorial.cxx-include-format
  :language: c++
  :start-at: #include <format>
  :end-at: #include <string>

.. literalinclude:: Step4/Tutorial/Tutorial.cxx
  :caption: TODO 4: Tutorial/Tutorial.cxx
  :name: Tutorial/Tutorial.cxx-format1
  :language: c++
  :start-at: if (argc < 2) {
  :end-at: return 1;
  :append: }
  :dedent: 2

.. literalinclude:: Step4/Tutorial/Tutorial.cxx
  :caption: TODO 5: Tutorial/Tutorial.cxx
  :name: Tutorial/Tutorial.cxx-format3
  :language: c++
  :start-at: // calculate square root
  :end-at: outputValue);
  :dedent: 2

.. raw:: html

  </details>

And again for the ``MathFunctions`` library.

.. raw:: html

  <details><summary>TODO 6-7: Click to show/hide answer</summary>

.. literalinclude:: Step4/MathFunctions/MathFunctions.cxx
  :caption: TODO 6: MathFunctions.cxx
  :name: MathFunctions/MathFunctions.cxx-include-format
  :language: c++
  :start-at: #include <format>
  :end-at: #include <iostream>

.. literalinclude:: Step4/MathFunctions/MathFunctions.cxx
  :caption: TODO 7: MathFunctions.cxx
  :name: MathFunctions/MathFunctions.cxx-format
  :language: c++
  :start-at: double delta
  :end-at: std::format
  :dedent: 4

.. raw:: html

  </details>

Exercise 3 - CMakePresets.json
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Managing these configuration values can quickly become overwhelming. In CI
systems it is appropriate to record these as part of a given CI step. For
example in a Github Actions CI step we might see something akin to the
following:

.. code-block:: yaml

  - name: Configure and Build
    run: |
      cmake \
        -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_STANDARD=20 \
        -DCMAKE_CXX_EXTENSIONS=ON \
        -DTUTORIAL_BUILD_UTILITIES=OFF \
        # Possibly many more options
        # ...

      cmake --build build

When developing code locally, typing all these options even once might be error
prone. If a fresh configuration is needed for any reason, doing so multiple
times could be exhausting.

There are many and varied solutions to this problem, and your choice is
ultimately up to your preferences as a developer. CLI-oriented developers
commonly use task runners to invoke CMake with their desired options for a
project. Most IDEs also have a custom mechanism for controlling CMake
configuration.

It would be impossible to fully enumerate every possible configuration workflow
here. Instead we will explore CMake's built-in solution, known as
:manual:`CMake Presets <cmake-presets(7)>`. Presets give us a format to name
and express collections of CMake configuration options.

.. note::
    Presets are capable of expressing entire CMake workflows, from
    configuration, through building, all the way to installing the software
    package.

    They are far more flexible than can we have room for here. We'll limit
    ourselves to using them for configuration.

CMake Presets come in two standard files, ``CMakePresets.json``, which is
intended to be a part of the project and tracked in source control; and
``CMakeUserPresets.json``, which is intended for local user configuration
and should not be tracked in source control.

The simplest preset which would be of use to a developer does nothing more
than configure variables.

.. code-block:: json

  {
    "version": 4,
    "configurePresets": [
      {
        "name": "example-preset",
        "cacheVariables": {
          "EXAMPLE_FOO": "Bar",
          "EXAMPLE_QUX": "Baz"
        }
      }
    ]
  }

When invoking CMake, where previously we would have done:

.. code-block:: console

  cmake -B build -DEXAMPLE_FOO=Bar -DEXAMPLE_QUX=Baz

We can now use the preset:

.. code-block:: console

  cmake -B build --preset example-preset

CMake will search for files named ``CMakePresets.json`` and
``CMakeUserPresets.json``, and load the named configuration from them if
available.

.. note::
  Command line flags can be mixed with presets. Command line flags have
  precedence over values found in a preset.

Presets also support limited macros, variables that can be brace-expanded
inside the preset. The only one of interest to us is the ``${sourceDir}`` macro,
which expands to the root directory of the project. We can use this to set our
build directory, skipping the :option:`-B <cmake -B>` flag when configuring
the project.

.. code-block:: json

  {
    "name": "example-preset",
    "binaryDir": "${sourceDir}/build"
  }

Goal
----

Configure and build the tutorial using a CMake Preset instead of command line
flags.

Helpful Resources
-----------------

* :manual:`cmake-presets(7)`

Files to Edit
-------------

* ``CMakePresets.json``

Getting Started
---------------

Continue to edit files from ``Step3``. Complete ``TODO 8`` and ``TODO 9``.

.. note::
  ``TODOs`` inside ``CMakePresets.json`` need to be *replaced*. There should
  be no ``TODO`` keys left inside the file when you have completed the exercise.

You can verify the preset is working correctly by deleting the existing build
folder before you configure, this will ensure you're not reusing the existing
CMake Cache for configuration.

.. note::
  On CMake 3.24 and newer, the same effect can be achieved by configuring with
  :option:`cmake --fresh`.

All future configuration changes will be via the ``CMakePresets.json`` file.

Build and Run
-------------

We can now use the preset file to manage our configuration.

.. code-block:: console

  cmake --preset tutorial

Presets are capable of running the build step for us, but for this tutorial
we'll continue to run the build ourselves.

.. code-block:: console

  cmake --build build

Solution
--------

There are two changes we need to make, first we want to set the build
directory (also called the "binary directory") to the ``build`` subdirectory
of our project folder, and second we need to set the ``CMAKE_CXX_STANDARD`` to
``20``.

.. raw:: html

  <details><summary>TODO 8-9: Click to show/hide answer</summary>

.. code-block:: json
  :caption: TODO 8-9: CMakePresets.json
  :name: CMakePresets.json-initial

  {
    "version": 4,
    "configurePresets": [
      {
        "name": "tutorial",
        "displayName": "Tutorial Preset",
        "description": "Preset to use with the tutorial",
        "binaryDir": "${sourceDir}/build",
        "cacheVariables": {
          "CMAKE_CXX_STANDARD": "20"
        }
      }
    ]
  }

.. raw:: html

  </details>
