file-download-range
-------------------

* Add the fields ``RANGE_START`` and ``RANGE_END`` to ``file(DOWNLOAD)``.
  Those fields provide a convenient way to specify the range, passed to the
  libcurl, which can be useful for downloading parts of big binary files.
