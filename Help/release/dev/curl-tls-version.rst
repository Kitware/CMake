curl-tls-version
----------------

* The :command:`file(DOWNLOAD)` and :command:`file(UPLOAD)` commands
  gained a ``TLS_VERSION <min>`` option to specify the minimum TLS
  version for connections to ``https://`` URLs.

* The :variable:`CMAKE_TLS_VERSION` variable and :envvar:`CMAKE_TLS_VERSION`
  environment variable were added to specify a default minimum TLS version
  for connections to ``https://`` URLs by the :command:`file(DOWNLOAD)`
  and :command:`file(UPLOAD)` commands.

* The :envvar:`CMAKE_TLS_VERIFY` environment variable was added as a fallback
  to the existing :variable:`CMAKE_TLS_VERIFY` variable.  It specifies
  whether to verify the server certificate for ``https://`` URLs by default.

* The :module:`ExternalProject` module's :command:`ExternalProject_Add`
  command gained a ``TLS_VERSION <min>`` option, and support for the
  :variable:`CMAKE_TLS_VERSION` variable and :envvar:`CMAKE_TLS_VERSION`
  environment variable, to specify the minimum TLS version for connections
  to ``https://`` URLs.

* The :command:`ctest_submit` command and :option:`ctest -T Submit <ctest -T>`
  step gained ``TLSVersion`` and ``TLSVerify`` options to control negotiation
  with ``https://`` URLs.  See the :variable:`CTEST_TLS_VERSION` and
  :variable:`CTEST_TLS_VERIFY` variables.
