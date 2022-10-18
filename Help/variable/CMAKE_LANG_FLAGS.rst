CMAKE_<LANG>_FLAGS
------------------

Flags for all build types.

``<LANG>`` flags used regardless of the value of :variable:`CMAKE_BUILD_TYPE`.

For each language, if this variable is not defined, it is initialized
and stored in the cache using values from environment variables in
combination with CMake's builtin defaults for the toolchain:

* ``CMAKE_C_FLAGS``:
  Initialized by the :envvar:`CFLAGS` environment variable.
* ``CMAKE_CXX_FLAGS``:
  Initialized by the :envvar:`CXXFLAGS` environment variable.
* ``CMAKE_CUDA_FLAGS``:
  Initialized by the :envvar:`CUDAFLAGS` environment variable.
* ``CMAKE_Fortran_FLAGS``:
  Initialized by the :envvar:`FFLAGS` environment variable.
* ``CMAKE_CSharp_FLAGS``:
  Initialized by the :envvar:`CSFLAGS` environment variable.
* ``CMAKE_HIP_FLAGS``:
  Initialized by the :envvar:`HIPFLAGS` environment variable.
* ``CMAKE_ISPC_FLAGS``:
  Initialized by the :envvar:`ISPCFLAGS` environment variable.

This value is a command-line string fragment. Therefore, multiple options
should be separated by spaces, and options with spaces should be quoted.

The flags in this variable will be passed to the compiler before those
in the per-configuration :variable:`CMAKE_<LANG>_FLAGS_<CONFIG>` variant,
and before flags added by the :command:`add_compile_options` or
:command:`target_compile_options` commands.
