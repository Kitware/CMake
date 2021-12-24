cuda-compiler-detection-robustness
----------------------------------

* CUDA compiler detection now issues an error in all cases when it's unable to
  compute the default architecture(s) if required (see :policy:`CMP0104`).

* CUDA compiler detection now correctly handles ``OFF`` for
  :variable:`CMAKE_CUDA_ARCHITECTURES` on Clang.

* CUDA compiler detection now supports the theoretical case of multiple default
  architectures.
