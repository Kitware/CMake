FPHSA-ExactCase-name
--------------------

* The :module:`FindPackageHandleStandardArgs` module
  ``FIND_PACKAGE_HANDLE_STANDARD_ARGS`` function now
  always populates the both ``<PackageName>_FOUND``
  and ``<UPPERCASE_NAME>_FOUND`` variables (the latter
  for backwards compatibility).  The ``FOUND_VAR``
  option is now ignored except to enforce its allowed
  values.
