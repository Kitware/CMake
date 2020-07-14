CMAKE_CUDA_HOST_COMPILER
------------------------

.. versionadded:: 3.10

When :variable:`CMAKE_CUDA_COMPILER <CMAKE_<LANG>_COMPILER>` is set to
NVIDIA ``nvcc``, ``CMAKE_CUDA_HOST_COMPILER`` selects the compiler
executable to use when compiling host code for ``CUDA`` language files.
This maps to the ``nvcc -ccbin`` option.

The ``CMAKE_CUDA_HOST_COMPILER`` variable may be set explicitly before CUDA is
first enabled by a :command:`project` or :command:`enable_language` command.
This can be done via ``-DCMAKE_CUDA_HOST_COMPILER=...`` on the command line
or in a :ref:`toolchain file <Cross Compiling Toolchain>`.  Or, one may set
the :envvar:`CUDAHOSTCXX` environment variable to provide a default value.

Once the CUDA language is enabled, the ``CMAKE_CUDA_HOST_COMPILER`` variable
is read-only and changes to it are undefined behavior.

.. note::

  Since ``CMAKE_CUDA_HOST_COMPILER`` is meaningful only when the
  ``CMAKE_CUDA_COMPILER`` is ``nvcc``, it does not make sense to
  set ``CMAKE_CUDA_HOST_COMPILER`` explicitly without also setting
  ``CMAKE_CUDA_COMPILER`` explicitly to be sure it is ``nvcc``.
