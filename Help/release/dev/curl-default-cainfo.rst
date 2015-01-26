curl-default-cainfo
-------------------

* When CMake is built with OpenSSL on systems other than Windows
  and OS X, commands supporting network communication via ``https``,
  such as :command:`file(DOWNLOAD)`, :command:`file(UPLOAD)`, and
  :command:`ctest_submit`, now search for OS-configured certificate
  authorities in a few ``/etc`` paths to be trusted automatically.
