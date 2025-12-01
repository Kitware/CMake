cli-tar-lzma
------------

* The :manual:`cmake(1)` ``-E tar`` tool supports support ``LZMA`` compression
  via ``--lzma`` option.
* The :command:`file(ARCHIVE_CREATE)` command's ``COMPRESSION`` option,
  supports ``LZMA`` and ``LZMA2`` compression. The ``LZMA2`` compression is
  an alias for ``XZ``.
* The :module:`CTestCoverageCollectGCOV` supports ``LZMA`` and ``LZMA2`` compression.
  The ``LZMA2`` compression is an alias for ``XZ``.
* The :module:`CTestCoverageCollectGCOV` supports ``LZMA`` and ``ZSTD`` compression
  file extensions for ``FROM_EXT`` compression mode.
