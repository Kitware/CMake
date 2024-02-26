CMAKE_TLS_VERSION
-----------------

.. versionadded:: 3.30

Specify the default value for the :command:`file(DOWNLOAD)` and
:command:`file(UPLOAD)` commands' ``TLS_VERSION`` option.
If this variable is not set, the commands check the
:envvar:`CMAKE_TLS_VERSION` environment variable.

The value may be one of:

* ``1.0``

* ``1.1``

* ``1.2``

* ``1.3``

This variable is also used by the :module:`ExternalProject` and
:module:`FetchContent` modules for internal calls to
:command:`file(DOWNLOAD)` and ``git clone``.
