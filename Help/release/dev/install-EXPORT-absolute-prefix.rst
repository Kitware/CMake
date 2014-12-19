install-EXPORT-absolute-prefix
------------------------------

* The :command:`install(EXPORT)` command now works with an absolute
  ``DESTINATION`` even if targets in the export set are installed
  with a destination or usage requirements specified relative to the
  install prefix.  The value of the :variable:`CMAKE_INSTALL_PREFIX`
  variable is hard-coded into the installed export file as the base
  for relative references.
