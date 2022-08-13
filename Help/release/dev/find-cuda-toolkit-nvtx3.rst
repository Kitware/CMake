find-cuda-toolkit-nvtx3
-----------------------

* The :module:`FindCUDAToolkit` module now provides a target for
  :ref:`nvtx3 <cuda_toolkit_nvtx3>` for CUDA 10.0+, which supersedes
  :ref:`nvToolsExt <cuda_toolkit_nvToolsExt>`. A deprecation warning is emitted
  when using ``nvToolsExt`` if the project requires CMake 3.25 and CUDA 10.0+
  is used.
