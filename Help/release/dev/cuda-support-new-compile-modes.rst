cuda-support-new-compile-modes
------------------------------

* A :prop_tgt:`CUDA_CUBIN_COMPILATION` target property was added to
  :ref:`Object Libraries` to support compiling to ``.cubin`` files
  instead of host object files. Currently only supported with NVIDIA.

* A :prop_tgt:`CUDA_FATBIN_COMPILATION` target property was added to
  :ref:`Object Libraries` to support compiling to ``.fatbin`` files
  instead of host object files. Currently only supported with NVIDIA.

* A :prop_tgt:`CUDA_OPTIX_COMPILATION` target property was added to
  :ref:`Object Libraries` to support compiling to ``.optixir`` files
  instead of host object files. Currently only supported with NVIDIA.
