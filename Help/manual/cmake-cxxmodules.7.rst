.. cmake-manual-description: CMake C++ Modules Support Reference

cmake-cxxmodules(7)
*******************

.. versionadded:: 3.28

C++ 20 introduced the concept of "modules" to the language.  The design
requires build systems to order compilations among each other to satisfy
``import`` statements reliably.  CMake's implementation asks the compiler
to scan source files for module dependencies during the build, collates
scanning results to infer ordering constraints, and tells the build tool
how to dynamically update the build graph.

Compiler Support
================

Compilers which CMake natively supports module dependency scanning include:

* MSVC toolset 14.34 and newer (provided with Visual Studio 17.4 and newer)
* LLVM/Clang 16.0 and newer
* GCC 14 (for the in-development branch, after 2023-09-20) and newer

Generator Support
=================

The list of generators which support scanning sources for C++ modules include:

- :generator:`Ninja`
- :generator:`Ninja Multi-Config`
- :generator:`Visual Studio 17 2022`

Limitations
-----------

There are a number of known limitations of the current C++ module support in
CMake.  This does not document known limitations or bugs in compilers as these
can change over time.

For all generators:

- Header units are not supported.
- No builtin support for ``import std;`` or other compiler-provided modules.

For the Ninja Generators:

- ``ninja`` 1.11 or newer is required.

For the :ref:`Visual Studio Generators`:

- Only Visual Studio 2022 and MSVC toolsets 14.34 (Visual Studio
  17.4) and newer.
- No support for exporting or installing BMI or module information.
- No diagnosis of using modules provided by ``PRIVATE`` sources from
  ``PUBLIC`` module sources.
