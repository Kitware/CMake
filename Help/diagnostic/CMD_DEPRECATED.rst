CMD_DEPRECATED
--------------

.. versionadded:: 4.4

.. diagnostic::
  :default: warn
  :parent: CMD_AUTHOR

  Warn about use of a deprecated function or package.  This is the category
  triggered by :command:`message(DEPRECATION)`.

  .. note::

    If :policy:`CMP0218` is not set to ``NEW``, :command:`message(DEPRECATION)`
    invocations, along with builtin deprecation messages that existed prior to
    CMake 4.4, will ignore this diagnostic state and will instead use the
    :variable:`CMAKE_WARN_DEPRECATED` and :variable:`CMAKE_ERROR_DEPRECATED`
    variables to determine the severity of deprecation messages.
