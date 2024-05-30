FindCUDAToolkit-search-paths
----------------------------

* The :module:`FindCUDAToolkit` module now searches the
  :variable:`CMAKE_CUDA_COMPILER <CMAKE_<LANG>_COMPILER>` and
  the environment variable :envvar:`CUDACXX` even when the CUDA
  language isn't enabled.
