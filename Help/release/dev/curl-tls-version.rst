curl-tls-version
----------------

* The :command:`file(DOWNLOAD)` and :command:`file(UPLOAD)` commands now
  require TLS 1.2 or higher for connections to ``https://`` URLs by default.
  See the :variable:`CMAKE_TLS_VERSION` variable for details.
