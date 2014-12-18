curl-darwinssl
--------------

* On OS X, commands supporting network communication, such as
  :command:`file(DOWNLOAD)`, :command:`file(UPLOAD)`, and
  :command:`ctest_submit`, now support SSL/TLS even when CMake
  is not built against OpenSSL.  The OS X native SSL/TLS
  implementation is used by default.  OS-configured certificate
  authorities will be trusted automatically.
