cmake-initial-cache-dirs
------------------------

* The :manual:`cmake(1)` ``-C <initial-cache>`` option now evaluates the
  initial cache script with :variable:`CMAKE_SOURCE_DIR` and
  :variable:`CMAKE_BINARY_DIR` set to the top-level source and build trees.
