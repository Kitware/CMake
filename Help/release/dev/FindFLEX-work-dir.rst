FindFLEX-work-dir
-----------------

* The :module:`FindFLEX` module's ``FLEX_TARGET`` command now runs ``flex``
  with :variable:`CMAKE_CURRENT_BINARY_DIR` as the working directory.
  See policy :policy:`CMP0098`.
