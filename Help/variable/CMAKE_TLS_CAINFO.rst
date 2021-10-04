CMAKE_TLS_CAINFO
----------------

Specify the default value for the :command:`file(DOWNLOAD)` and
:command:`file(UPLOAD)` commands' ``TLS_CAINFO`` options.
It is unset by default.

This variable is also used by the :module:`ExternalProject` module
for internal calls to :command:`file(DOWNLOAD)`.
