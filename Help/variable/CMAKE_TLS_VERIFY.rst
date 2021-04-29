CMAKE_TLS_VERIFY
----------------

Specify the default value for the :command:`file(DOWNLOAD)` and
:command:`file(UPLOAD)` commands' ``TLS_VERIFY`` options.
If not set, the default is *off*.

This setting is also used by the :module:`ExternalProject` module
for internal calls to :command:`file(DOWNLOAD)`.

TLS verification can help provide confidence that one is connecting
to the desired server.  When downloading known content, one should
also use file hashes to verify it.

.. code-block:: cmake

  set(CMAKE_TLS_VERIFY TRUE)
