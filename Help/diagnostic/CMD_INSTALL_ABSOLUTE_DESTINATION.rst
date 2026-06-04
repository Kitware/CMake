CMD_INSTALL_ABSOLUTE_DESTINATION
--------------------------------

.. versionadded:: 4.4

.. diagnostic::
  :default: ignore
  :parent: CMD_AUTHOR

  Warn when an :command:`install` command specifies an absolute ``DESTINATION``
  path.  Absolute destinations are typically undesirable because they prevent
  the installation prefix from being overridden at install time.
