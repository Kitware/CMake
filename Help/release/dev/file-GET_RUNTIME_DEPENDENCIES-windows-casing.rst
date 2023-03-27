file-GET_RUNTIME_DEPENDENCIES-windows-casing
--------------------------------------------


* The :command:`file(GET_RUNTIME_DEPENDENCIES)` command now case-preserves
  DLL names reported on Windows.  They are still converted to lowercase
  for filter matching.
