CMAKE_<LANG>_HOST_COMPILER_VERSION
----------------------------------

.. versionadded:: 3.31

This variable is available when ``<LANG>`` is ``CUDA`` or ``HIP``
and :variable:`CMAKE_<LANG>_COMPILER_ID` is ``NVIDIA``.
It contains the version of the host compiler invoked by ``nvcc``,
either by default or as specified by :variable:`CMAKE_<LANG>_HOST_COMPILER`,
in the same format as :variable:`CMAKE_<LANG>_COMPILER_VERSION`.

.. versionadded:: 4.5
  This variable is also available when ``<LANG>`` is ``SYCL`` and
  :variable:`CMAKE_<LANG>_COMPILER_ID` is ``AdaptiveCpp``.  It contains the
  underlying host compiler's version; :variable:`CMAKE_<LANG>_COMPILER_VERSION`
  contains the AdaptiveCpp release version.

  .. note::
    SYCL support is experimental and gated by ``CMAKE_EXPERIMENTAL_SYCL``.
