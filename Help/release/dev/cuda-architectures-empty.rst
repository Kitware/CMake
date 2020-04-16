cuda-architectures-empty
------------------------

* :variable:`CMAKE_CUDA_ARCHITECTURES` is now initialized when
  :variable:`CMAKE_CUDA_COMPILER_ID <CMAKE_<LANG>_COMPILER_ID>` is ``NVIDIA``.
  Empty :prop_tgt:`CUDA_ARCHITECTURES` raises an error. See policy
  :policy:`CMP0104`.
