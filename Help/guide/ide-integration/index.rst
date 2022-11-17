IDE Integration Guide
*********************

.. only:: html

  .. contents::

Introduction
============

Integrated development environments (IDEs) may want to integrate with CMake to
improve the development experience for CMake users. This document lays out the
recommended best practices for such integration.

Bundling
========

Many IDE vendors will want to bundle a copy of CMake with their IDE. IDEs that
bundle CMake should present the user with the option of using an external CMake
installation instead of the bundled one, in case the bundled copy becomes
outdated and the user wants to use a newer version.

While IDE vendors may be tempted to bundle different versions of CMake with
their application, such practice is not recommended. CMake has strong
guarantees of backwards compatibility, and there is no reason not to use a
newer version of CMake than what a project requires, or indeed, the very latest
version. Therefore, it is recommended that IDE vendors that bundle CMake with
their application always include the very latest patch version of CMake
available at the time of release.

As a suggestion, IDEs may also ship a copy of the Ninja buildsystem alongside
CMake. Ninja is highly performant and well-supported on all platforms that
support CMake. IDEs that bundle Ninja should use Ninja 1.10 or later, which
contains features needed to support Fortran builds.

Presets
=======

CMake supports a file format called ``CMakePresets.json``, and its
user-specific counterpart, ``CMakeUserPresets.json``. This file contains
information on the various configure presets that a user may want. Each preset
may have a different compiler, build flags, etc. The details of this format are
explained in the :manual:`cmake(1)` manual.

IDE vendors are encouraged to read and evaluate this file the same way CMake
does, and present the user with the presets listed in the file. Users should be
able to see (and possibly edit) the CMake cache variables, environment
variables, and command line options that are defined for a given preset. The
IDE should then construct the list of appropriate :manual:`cmake(1)` command
line arguments based on these settings, rather than using the
:option:`--preset= <cmake --preset>` option directly. The
:option:`--preset= <cmake --preset>` option is intended only as a convenient
frontend for command line users, and should not be used by the IDE.

For example, if a preset named ``ninja`` specifies ``Ninja`` as the generator
and ``${sourceDir}/build`` as the build directory, instead of running:

.. code-block:: console

  cmake -S /path/to/source --preset=ninja

the IDE should instead calculate the settings of the ``ninja`` preset, and then
run:

.. code-block:: console

  cmake -S /path/to/source -B /path/to/source/build -G Ninja

In cases where a preset contains lots of cache variables, and passing all of
them as :option:`-D <cmake -D>` flags would cause the command line length limit
of the platform to be exceeded, the IDE should instead construct a temporary
cache script and pass it with the :option:`-C <cmake -C>` flag.

While reading, parsing, and evaluating the contents of ``CMakePresets.json`` is
straightforward, it is not trivial. In addition to the documentation, IDE
vendors may also wish to refer to the CMake source code and test cases for a
better understanding of how to implement the format.
:download:`This file <../../manual/presets/schema.json>` provides a
machine-readable JSON schema for the ``CMakePresets.json`` format that IDE
vendors may find useful for validation and providing editing assistance.

Configuring
===========

IDEs that invoke :manual:`cmake(1)` to run the configure step may wish to
receive information about the artifacts that the build will produce, as well
as the include directories, compile definitions, etc. used to build the
artifacts. Such information can be obtained by using the
:manual:`File API <cmake-file-api(7)>`. The manual page for the File API
contains more information about the API and how to invoke it.
:manual:`Server mode <cmake-server(7)>` was removed as of CMake 3.20 and
should not be used on CMake 3.14 or later.

IDEs should avoid creating more build trees than necessary, and only create
multiple build trees if the user wishes to switch to a different compiler,
use different compile flags, etc. In particular, IDEs should NOT create
multiple single-config build trees which all have the same properties except
for a differing :variable:`CMAKE_BUILD_TYPE`, effectively creating a
multi-config environment. Instead, the :generator:`Ninja Multi-Config`
generator, in conjunction with the :manual:`File API <cmake-file-api(7)>` to
get the list of build configurations, should be used for this purpose.

IDEs should not use the "extra generators" with Makefile or Ninja generators,
which generate IDE project files in addition to the Makefile or Ninja files.
Instead the :manual:`File API <cmake-file-api(7)>` should be used to get the
list of build artifacts.

Building
========

If a Makefile or Ninja generator is used to generate the build tree, it is not
recommended to invoke ``make`` or ``ninja`` directly. Instead, it is
recommended that the IDE invoke :manual:`cmake(1)` with the
:option:`--build <cmake --build>` argument, which will in turn invoke the
appropriate build tool.

If an IDE project generator is used, such as :generator:`Xcode` or one of the
Visual Studio generators, and the IDE understands the project format used, the
IDE should read the project file and build it the same way it would otherwise.

The :manual:`File API <cmake-file-api(7)>` can be used to obtain a list of
build configurations from the build tree, and the IDE should present this list
to the user to select a build configuration.

Testing
=======

:manual:`ctest(1)` supports outputting a JSON format with information about the
available tests and test configurations. IDEs which want to run CTest should
obtain this information and use it to present the user with a list of tests.

IDEs should not invoke the ``test`` target of the generated buildsystem.
Instead, they should invoke :manual:`ctest(1)` directly.

IDEs with CMake integration
===========================

The following IDEs support CMake natively:

* `CLion`_
* `KDevelop`_
* `QtCreator`_
* `Vim`_ (via a plugin)
* `Visual Studio`_
* `VSCode`_ (via a plugin)

.. _CLion: https://www.jetbrains.com/clion/
.. _KDevelop: https://www.kdevelop.org/
.. _QtCreator: https://www.qt.io/product/development-tools
.. _Vim: https://www.vim.org/
.. _Visual Studio: https://visualstudio.microsoft.com/
.. _VSCode: https://code.visualstudio.com/

Additionally, CMake has builtin support for some IDEs:

* :ref:`IDE Build Tool Generators`:
  Generate IDE native build systems such as Visual Studio or Xcode.
* :ref:`Extra Generators`:
  Extend :ref:`Command-Line Build Tool Generators` to generate IDE
  project files that hook into the command-line build system.
  Superseded by the :manual:`File API <cmake-file-api(7)>`.
