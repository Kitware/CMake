WriteCompilerDetectionHeader thread_local portability
-----------------------------------------------------

* The :module:`WriteCompilerDetectionHeader` module learned to
  create a define for portability of the cxx_thread_local feature. The define
  expands to either the C++11 ``thread_local`` keyword, or a
  pre-standardization compiler-specific equivalent, as appropriate.
