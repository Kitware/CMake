curl-http2
----------

* When building CMake itself from source and not using a system-provided
  libcurl, HTTP/2 support is now enabled for commands supporting
  network communication via ``http(s)``, such as :command:`file(DOWNLOAD)`,
  :command:`file(UPLOAD)`, and :command:`ctest_submit`.
  The precompiled binaries provided on ``cmake.org`` now support HTTP/2.
