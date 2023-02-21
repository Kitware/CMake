find_package-PACKAGENAME_ROOT
-----------------------------

* The :command:`find_package` command now searches prefixes specified by
  upper-case :variable:`<PACKAGENAME>_ROOT` CMake variables and upper-case
  :envvar:`<PACKAGENAME>_ROOT` environment variables.
  See policy :policy:`CMP0144`.
