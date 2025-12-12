cli-tar-7z-algorithms
---------------------

* The :manual:`cmake(1)` ``-E tar`` tool supports support compression methods
  specification for ``7zip`` and ``zip`` formats.
* The :manual:`cmake(1)` ``-E tar`` tool supports support compression levels
  specification for ``7zip`` and ``zip`` formats.
* The :command:`file(ARCHIVE_CREATE)` command's ``COMPRESSION`` option,
  supports ``Deflate`` compression. The ``Deflate`` compression is
  an alias for ``GZip``.
