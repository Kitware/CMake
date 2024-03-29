CMAKE_TLS_VERIFY
----------------

.. versionadded:: 3.30

.. include:: ENV_VAR.txt

Specify the default value for the :command:`file(DOWNLOAD)` and
:command:`file(UPLOAD)` commands' ``TLS_VERIFY`` option.
This environment variable is used if the option is not given
and the :variable:`CMAKE_TLS_VERIFY` cmake variable is not set.
