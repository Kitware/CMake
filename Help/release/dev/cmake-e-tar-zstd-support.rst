Help/release/dev/cmake-e-tar-zstd-support
-----------------------------------------

* The :manual:`cmake(1)` ``-E tar`` tool now support Zstandard compression
  algorithm with ``--zstd`` option. Zstandard was designed to give
  a compression ratio comparable to that of the DEFLATE (zip) algorithm,
  but faster, especially for decompression.
