parallel-lzma-compression
-------------------------

* The :cpack_gen:`CPack Archive Generator`'s ``TXZ`` format learned the
  :variable:`CPACK_ARCHIVE_THREADS` variable to enable parallel compression.
  Requires support in the ``liblzma`` used by CMake.
