curl-tls-version
----------------

* The :command:`file(DOWNLOAD)` and :command:`file(UPLOAD)` commands now
  require TLS 1.2 or higher for connections to ``https://`` URLs by default.
  See the :variable:`CMAKE_TLS_VERSION` variable for details.

* The :command:`ctest_submit` command and :option:`ctest -T Submit <ctest -T>`
  step now require TLS 1.2 or higher for connections to ``https://`` URLs by
  default.  See the :variable:`CTEST_TLS_VERSION` variable for details.
