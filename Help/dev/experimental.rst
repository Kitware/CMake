CMake Experimental Features Guide
*********************************

The following is a guide to CMake experimental features that are
under development and not yet included in official documentation.
See documentation on `CMake Development`_ for more information.

.. _`CMake Development`: README.rst

Features are gated behind ``CMAKE_EXPERIMENTAL_`` variables which must be set
to specific values in order to enable their gated behaviors. Note that the
specific values will change over time to reinforce their experimental nature.
When used, a warning will be generated to indicate that an experimental
feature is in use and that the affected behavior in the project is not part of
CMake's stability guarantees.

C++20 Module APIs
=================

Variable: ``CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API``
Value: ``aa1f7df0-828a-4fcd-9afc-2dc80491aca7``

In order to support C++20 modules, there are a number of behaviors that have
CMake APIs to provide the required features to build and export them from a
project.

Limitations
-----------

There are a number of known limitations of the current C++20 module support in
CMake.  This does not document known limitations or bugs in compilers as these
can change over time.

For all generators:

- Only in-project modules may be used.  While there is some support for
  exporting module information, there is no mechanism for using it at the
  moment.

For the Ninja Generators:

- ``ninja`` 1.10 or newer is required.

For the Visual Studio Generators:

- Only Visual Studio 2022 and toolchains newer than 19.34 (Visual Studio
  17.4).
- No support for exporting or installing BMI or module information.
- No diagnosis of using modules provided by ``PRIVATE`` sources from
  ``PUBLIC`` module sources.

C++20 Module Dependencies
=========================

The Ninja generator has experimental infrastructure supporting C++20 module
dependency scanning.  This is similar to the Fortran modules support, but
relies on external tools to scan C++20 translation units for module
dependencies.  The approach is described by Kitware's `D1483r1`_ paper.

In order to activate CMake's experimental support for C++20 module
dependencies, set the following variables:

``CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API``
  Set this to the UUID documented above.

Some compilers already have support for module dependency scanning:

* MSVC 19.34 and newer (provided with Visual Studio 17.4 and newer)
* LLVM/Clang 16.0 and newer

For those, only the above variables need to be set by project code.
For compilers with in-development support, additional variables must
be set as follows.

``CMAKE_EXPERIMENTAL_CXX_SCANDEP_SOURCE``
  Set this to tell CMake how to invoke the C++20 module dependency
  scanning tool.

``CMAKE_EXPERIMENTAL_CXX_MODULE_MAP_FORMAT``
  Set this for compilers that generate module maps.  See below.

``CMAKE_EXPERIMENTAL_CXX_MODULE_MAP_FLAG``
  Set this for compilers that generate module maps.  See below.

For example, add code like the following to a test project:

.. code-block:: cmake

  string(CONCAT CMAKE_EXPERIMENTAL_CXX_SCANDEP_SOURCE
    "<CMAKE_CXX_COMPILER> <DEFINES> <INCLUDES> <FLAGS> <SOURCE>"
    " -MT <DYNDEP_FILE> -MD -MF <DEP_FILE>"
    " ${flags_to_scan_deps} -fdep-file=<DYNDEP_FILE> -fdep-output=<OBJECT>"
    )

The tool specified by ``CMAKE_EXPERIMENTAL_CXX_SCANDEP_SOURCE`` is
expected to process the translation unit, write preprocessor dependencies
to the file specified by the ``<DEP_FILE>`` placeholder, and write module
dependencies to the file specified by the ``<DYNDEP_FILE>`` placeholder. The
``CMAKE_EXPERIMENTAL_CXX_SCANDEP_DEPFILE_FORMAT`` file may be set to ``msvc``
for scandep rules which use ``msvc``-style dependency reporting.

The module dependencies should be written in the format described
by the `P1689r5`_ paper.

Compiler writers may try out their scanning functionality using
the `cxx-modules-sandbox`_ test project, modified to set variables
as above for their compiler.

For compilers that generate module maps, tell CMake as follows:

.. code-block:: cmake

  set(CMAKE_EXPERIMENTAL_CXX_MODULE_MAP_FORMAT "gcc")
  set(CMAKE_EXPERIMENTAL_CXX_MODULE_MAP_FLAG
    "${compiler_flags_for_module_map} -fmodule-mapper=<MODULE_MAP_FILE>")

Currently, the only supported formats are, ``clang``, ``gcc``, and ``msvc``.
The ``gcc`` format is described in the GCC documentation, but the relevant
section for the purposes of CMake is:

    A mapping file consisting of space-separated module-name, filename
    pairs, one per line.  Only the mappings for the direct imports and any
    module export name need be provided.  If other mappings are provided,
    they override those stored in any imported CMI files.  A repository
    root may be specified in the mapping file by using ``$root`` as the
    module name in the first active line.

    -- GCC module mapper documentation

The ``msvc`` format is a response file containing flags required to compile
any module interfaces properly as well as find any required files to satisfy
``import`` statements as required for Microsoft's Visual Studio toolchains.

Similarly, the ``clang`` format is a response file containing flags using
Clang's module flags.

.. _`D1483r1`: https://mathstuf.fedorapeople.org/fortran-modules/fortran-modules.html
.. _`P1689r5`: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p1689r5.html
.. _`cxx-modules-sandbox`: https://github.com/mathstuf/cxx-modules-sandbox
