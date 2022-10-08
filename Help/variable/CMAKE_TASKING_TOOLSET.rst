CMAKE_TASKING_TOOLSET
---------------------

.. versionadded:: 3.25

Select the Tasking toolset which provides the compiler

Architecture compilers are provided by different toolchains with
incompatible versioning schemes.  Set this variable in a
:variable:`toolchain file <CMAKE_TOOLCHAIN_FILE>` so CMake can detect
the compiler and version correctly. If no toolset is specified,
``Standalone`` is assumed.

Projects that can be built with different architectures and/or toolsets must
take :variable:`CMAKE_TASKING_TOOLSET` and
:variable:`CMAKE_<LANG>_COMPILER_ARCHITECTURE_ID` into account to qualify
:variable:`CMAKE_<LANG>_COMPILER_VERSION`.

``TriCore``
  Compilers are provided by the TriCore toolset.

``SmartCode``
  Compilers are provided by the SmartCode toolset.

``Standalone``
  Compilers are provided by the standalone toolsets.

  .. note::

    For the TriCore architecture, the compiler from the TriCore toolset is
    selected as standalone compiler.
