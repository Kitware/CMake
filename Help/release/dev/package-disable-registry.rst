package-disable-registry
------------------------

* The :command:`export(PACKAGE)` command learned to check the
  :variable:`CMAKE_EXPORT_NO_PACKAGE_REGISTRY` variable to skip
  exporting the package.

* The :command:`find_package` command learned to check the
  :variable:`CMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY` and
  :variable:`CMAKE_FIND_PACKAGE_NO_SYSTEM_PACKAGE_REGISTRY`
  variables to skip searching the package registries.
