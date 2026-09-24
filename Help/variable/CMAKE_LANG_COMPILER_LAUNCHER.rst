CMAKE_<LANG>_COMPILER_LAUNCHER
------------------------------

.. versionadded:: 3.4

Default value for :prop_tgt:`<LANG>_COMPILER_LAUNCHER` target property.
This variable is used to initialize the property on each target as it is
created.  This is done only when ``<LANG>`` is ``C``, ``CXX``, ``Fortran``,
``HIP``, ``ISPC``, ``OBJC``, ``OBJCXX``, ``CUDA``, or ``SYCL``.

.. versionchanged:: 4.5
  Added support for ``SYCL``.

  .. note::
    SYCL support is experimental and gated by ``CMAKE_EXPERIMENTAL_SYCL``.

This variable is initialized to the :envvar:`CMAKE_<LANG>_COMPILER_LAUNCHER`
environment variable if it is set.
