cpack-zstd-parallel
-------------------

* CPack now supports the :variable:`CPACK_THREADS` option for ``zstd``
  compression when compiled with libarchive 3.6 or higher.  It is
  supported by official CMake binaries available on ``cmake.org``.
