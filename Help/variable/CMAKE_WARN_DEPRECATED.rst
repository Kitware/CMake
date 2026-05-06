CMAKE_WARN_DEPRECATED
---------------------

.. deprecated:: 4.4

Whether to issue warnings for deprecated functionality.

This is a deprecated mechanism for interacting with the
:diagnostic:`CMD_DEPRECATED` :manual:`diagnostic <cmake-diagnostics(7)>`.
If policy :policy:`CMP0218` is set to ``NEW``, this variable is ignored.

If policy :policy:`CMP0218` is not set to ``NEW``, the :command:`message`
command's ``DEPRECATION`` message type will use
``CMAKE_WARN_DEPRECATED`` and :variable:`CMAKE_ERROR_DEPRECATED`
to determine the severity of a deprecation diagnostic.  The severity will be:

* ``IGNORE``, if ``CMAKE_WARN_DEPRECATED`` is ``OFF`` and
  ``CMAKE_ERROR_DEPRECATED`` is unset or ``OFF``.

* ``WARN``, if ``CMAKE_WARN_DEPRECATED`` is unset or ``ON`` and
  ``CMAKE_ERROR_DEPRECATED`` is unset or ``OFF``.

* ``FATAL_ERROR`` if ``CMAKE_ERROR_DEPRECATED`` is ``ON``.

Setting ``CMAKE_WARN_DEPRECATED`` in the cache will alter the default state of
the :diagnostic:`CMD_DEPRECATED` :manual:`diagnostic <cmake-diagnostics(7)>`;
however, the :option:`-W[no-][error=]deprecated <cmake -W>` option, and
:preset:`warnings.deprecated <configurePresets.warnings.deprecated>` and/or
:preset:`errors.deprecated <configurePresets.errors.deprecated>` preset
fields, will take precedence. CMake will also update the cached value of
``CMAKE_WARN_DEPRECATED`` to reflect the actual state of the diagnostic as of
the start of script execution.
