cuda-device-lto
---------------

* ``CUDA`` language now supports device link time optimization when using
  ``nvcc``. The :variable:`CMAKE_INTERPROCEDURAL_OPTIMIZATION` variable and
  the associated :prop_tgt:`INTERPROCEDURAL_OPTIMIZATION` target property will
  activate device LTO.
