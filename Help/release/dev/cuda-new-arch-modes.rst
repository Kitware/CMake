cuda-new-arch-modes
-------------------

* The :prop_tgt:`CUDA_ARCHITECTURES` target property now supports the
  `all`, and `all-major` values when the CUDA compiler id is ``NVIDIA``,
  and version is 11.5+.

* The :variable:`CMAKE_CUDA_ARCHITECTURES` variable now supports the
  `all`, and `all-major` values when the `CUDA` compiler id is ``NVIDIA``,
  and version is 11.5+.
