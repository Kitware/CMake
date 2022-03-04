cuda-arch-native
----------------

* The :variable:`CMAKE_CUDA_ARCHITECTURES` variable and associated
  :prop_tgt:`CUDA_ARCHITECTURES` target property now support the
  special ``native`` value to compile for the architectures(s)
  of the host's GPU(s).
