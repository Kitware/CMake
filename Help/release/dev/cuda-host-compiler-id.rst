cuda-host-compiler-id
---------------------

* The :variable:`CMAKE_<LANG>_HOST_COMPILER_ID` and
  :variable:`CMAKE_<LANG>_HOST_COMPILER_VERSION` variables were added,
  where ``<LANG>`` is either ``CUDA`` or ``HIP``.  They are populated
  when :variable:`CMAKE_<LANG>_COMPILER_ID` is ``NVIDIA`` to identify
  NVCC's host compiler.
