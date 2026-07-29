CMD_NON_TARGET_DIRECTIVE
------------------------

.. versionadded:: 4.5

.. diagnostic::
  :default: ignore
  :parent: CMD_STRICT

  Warn about use of non-target directives which alter the build environment.
  Unencapsulated alterations to the build environment can have unintentionally
  broad effect, potentially resulting in unnecessary build dependencies.
  Additionally, such commands cannot be used to express interface usage
  requirements.  It is recommended that new development use target-specific
  commands instead.
