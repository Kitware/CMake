curl-tls-version
----------------

* The :command:`file(DOWNLOAD)` and :command:`file(UPLOAD)` commands
  gained a ``TLS_VERSION <min>`` option to specify the minimum TLS
  version for connections to ``https://`` URLs.

* The :variable:`CMAKE_TLS_VERSION` variable and :envvar:`CMAKE_TLS_VERSION`
  environment variable were added to specify a default minimum TLS version
  for connections to ``https://`` URLs by the :command:`file(DOWNLOAD)`
  and :command:`file(UPLOAD)` commands.

* The :module:`ExternalProject` module's :command:`ExternalProject_Add`
  command gained a ``TLS_VERSION <min>`` option, and support for the
  :variable:`CMAKE_TLS_VERSION` variable and :envvar:`CMAKE_TLS_VERSION`
  environment variable, to specify the minimum TLS version for connections
  to ``https://`` URLs.
