curl-tls-version
----------------

* The :command:`file(DOWNLOAD)` and :command:`file(UPLOAD)` commands
  gained a ``TLS_VERSION <min>`` option to specify the minimum TLS
  version for connections to ``https://`` URLs.

* The :variable:`CMAKE_TLS_VERSION` variable was added to specify a
  default minimum TLS version for connections to ``https://`` URLs by
  the :command:`file(DOWNLOAD)` and :command:`file(UPLOAD)` commands.
