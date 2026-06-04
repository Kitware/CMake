CMD_UNUSED_CLI
--------------

.. versionadded:: 4.4

.. diagnostic::
  :default: ignore

  Warn about variables that are declared on the command line, but not used.

  This diagnostic is issued outside of the configuration / generation phases.
  Accordingly, while the action of this warning category can be queried as
  usual, the :command:`cmake_diagnostic` command is unable to affect the action
  of this diagnostic.
