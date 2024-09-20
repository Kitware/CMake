curl-tls-verify
---------------

* The :command:`file(DOWNLOAD)` and :command:`file(UPLOAD)` commands now
  verify TLS server certificates for connections to ``https://`` URLs by
  default.  See the :variable:`CMAKE_TLS_VERIFY` variable for details.
  This change was made without a policy so that users are protected
  even when building projects that have not been updated.
  Users may set the :envvar:`CMAKE_TLS_VERIFY` environment
  variable to ``0`` to restore the old default.

* The :command:`ctest_submit` command and :option:`ctest -T Submit <ctest -T>`
  step now verify TLS server certificates for connections to ``https://`` URLs
  by default.  See the :variable:`CTEST_TLS_VERIFY` variable for details.
