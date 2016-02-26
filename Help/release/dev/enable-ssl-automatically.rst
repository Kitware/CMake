enable-ssl-automatically
------------------------

* On UNIX platforms, when building CMake itself from source and not using a
  system-provided libcurl, OpenSSL is now used by default if it is found on
  the system.  This enables SSL/TLS support for commands supporting network
  communication via ``https``, such as :command:`file(DOWNLOAD)`,
  :command:`file(UPLOAD)`, and :command:`ctest_submit`.
