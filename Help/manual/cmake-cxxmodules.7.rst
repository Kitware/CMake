.. cmake-manual-description: CMake C++ Modules Support Reference

cmake-cxxmodules(7)
*******************

.. versionadded:: 3.28

C++ 20 introduced the concept of ":term:`modules <C++ module>`" to the
language.  The design requires :term:`build systems <build system>` to order
compilations to satisfy ``import`` statements reliably.  CMake's
implementation asks the compiler to scan source files for module dependencies
during the build, collates scanning results to infer ordering constraints, and
tells the :term:`build tool` how to dynamically update the
build graph.

Compilation Strategy
====================

With C++ modules, compiling a set of C++ sources is no longer embarrassingly
parallel.  That is, any given source may require the compilation of another
source file first in order to provide a "BMI" (or "CMI") that C++ compilers
use to satisfy ``import`` statements in other sources.  With included headers,
sources could share their declarations so that any consumers could compile
independently. With modules, declarations are now generated into these BMI
files by the compiler during compilation based on the contents of the source
file and its ``export`` statements.  That means that, in order to get a
correct build without regenerating the build graph via a configure and
generate phase for every source change, the ordering needs to be extracted
from the source during the build phase.

:term:`Build systems <build system>` must be able to order these compilations
within the build graph.  There are multiple strategies that are suitable for
this, but each has advantages and disadvantages.  CMake uses a "scanning" step
strategy, which is the most visible modules-related change for CMake users in
the context of the build.  CMake provides multiple ways to control the
scanning behavior of source files.

.. _`P1689R5`: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p1689r5.html

.. note::

   CMake is focusing on correct builds before looking at performance
   improvements. There are known tactics within the chosen strategy which may
   offer build performance improvements. However, they are being deferred
   until we have a working model against which to compare them. It is also
   important to note that a tactic useful in one situation (e.g., clean
   builds) may not be performant in a different situation (e.g., incremental
   builds). Finding a balance and offering controls to select the tactics is
   future work.

Scanning Control
================

Whether or not sources get scanned for C++ module usage is dependent on the
following queries.  The first query that provides a decision of whether to
scan or not is used.

- If the source file belongs to a file set of type ``CXX_MODULES``, it will
  be scanned.
- If the target does not use at least C++ 20, it will not be scanned.
- If the source file is not the language ``CXX``, it will not be scanned.
- If the :prop_sf:`CXX_SCAN_FOR_MODULES` source file property is set, its
  value will be used.
- If the :prop_tgt:`CXX_SCAN_FOR_MODULES` target property is set, its value
  will be used.  Set the :variable:`CMAKE_CXX_SCAN_FOR_MODULES` variable
  to initialize this property on all targets as they are created.
- Otherwise, the source file will be scanned if the compiler and generator
  support scanning.  See policy :policy:`CMP0155`.

Note that any scanned source will be excluded from any unity build (see
:prop_tgt:`UNITY_BUILD`) because module-related statements can only happen at
one place within a C++ translation unit.

Compiler Support
================

The list of compilers for which CMake supports scanning sources for C++
modules includes:

* MSVC toolset 14.34 and newer (provided with Visual Studio 17.4 and newer)
* LLVM/Clang 16.0 and newer
* GCC 14 and newer

``import std`` Support
======================

Support for ``import std`` is limited to the following toolchain and standard
library combinations:

* Clang 18.1.2 and newer with ``-stdlib=libc++`` or ``-stdlib=libstdc++``
* MSVC toolset 14.36 and newer (provided with Visual Studio 17.6 Preview 2 and
  newer)
* GCC 15 and newer

The :variable:`CMAKE_CXX_COMPILER_IMPORT_STD` variable lists standard levels
which have support for ``import std`` in the active C++ toolchain.

.. note::

   This support is provided only when experimental support for
   ``import std`` has been enabled by the
   ``CMAKE_EXPERIMENTAL_CXX_IMPORT_STD`` gate.

Generator Support
=================

The list of generators which support scanning sources for C++ modules
includes:

- :generator:`Ninja`
- :generator:`Ninja Multi-Config`
- :generator:`Visual Studio 17 2022`
- :generator:`Visual Studio 18 2026`

Note that the :ref:`Ninja Generators` require ``ninja`` 1.11 or newer.

Limitations
-----------

There are a number of known limitations of the current C++ module support in
CMake.  Known limitations or bugs in compilers are not listed here, as these
can change over time.

For all generators:

- Header units are not supported.
- There is no builtin support for ``import std`` or other compiler-provided
  modules.

For the :ref:`Visual Studio Generators`:

- Only Visual Studio 2022 and MSVC toolsets 14.34 (Visual Studio
  17.4) and newer are supported.
- Exporting or installing BMI or module information is not supported.
- Compiling BMIs from ``IMPORTED`` targets with C++ modules (including
  ``import std``) is not supported.
- Use of modules provided by ``PRIVATE`` sources from ``PUBLIC`` module
  sources is not diagnosed.

Possible Future Enhancements
============================

This section documents possible future enhancements to CMake's support of C++
modules.  Nothing here is a guarantee of future implementation, and the
ordering is arbitrary.

Batch Scanning
--------------

It is possible to scan all sources within a target at once, which should be
faster when sources share transitive includes.  This does have side effects
for incremental builds, as the update of any source in the target means that
all sources in the target are scanned again.  Given how much faster scanning
can be, it should be negligible to do such "extra" scanning assuming that
unchanged results do not trigger recompilations.

BMI Modification Optimization
-----------------------------

Currently, as with object files, compilers always update a BMI file even if
the contents have not changed.  Because modules increase the potential scope
of "non-changes" to cause (conceptually) unnecessary recompilation, it might
be useful to avoid recompilation of module consumers if the BMI file has not
changed.  This might be achieved by wrapping the compilation to juggle the BMI
through a ``cmake -E copy_if_different`` pass with ``ninja``'s ``restat = 1``
feature to avoid recompiling importers if the BMI file doesn't actually
change.

.. _`easier-source-specification`:

Easier Source Specification
---------------------------

The initial implementation of CMake's module support had used the "just list
sources; CMake will figure it out" pattern.  However, this ran into issues
related to other metadata requirements.  These were discovered while
implementing CMake support beyond just building the modules-using code.

Conflicts with `Separate BMI Generation <separate-bmi-generation_>`__ on a
single target, as that requires knowledge of all BMI-generating rules at
generate time.

.. _`separate-bmi-generation`:

Separate BMI Generation
-----------------------

CMake currently uses a single rule to generate both the BMI and the object
file for a compilation.  At least Clang supports compiling an object directly
from the BMI.  This would be beneficial because BMI generation is typically
faster than compilation and generating the BMI as a separate step allows
importers to start compiling without waiting for the object to also be
generated.

This is not supported in the current implementation as only Clang supports
generating an object directly from the BMI.  Other compilers either do not
support such a two-phase generation (GCC) or need to start object compilation
from the source again.

Conflicts with `Easier Source Specification <easier-source-specification_>`__
on a single target because CMake must know all BMI-generating sources at
generate time rather than build time to create the two-phase rules.

Module Compilation Glossary
===========================

.. glossary::

   build system
     A tool that facilitates the building of software which includes a model
     of how components of the build relate to each other.  For example, CMake,
     Meson, build2, and more.

   build tool
     A build graph execution tool.  For example, `ninja` and `make`.  Some
     build tools are also their own :term:`build system`.

   C++ module
     A C++20 language feature for describing the API of a piece of software.
     Intended as a replacement for headers for this purpose.
