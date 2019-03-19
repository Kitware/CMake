export-package-default-off
--------------------------

* The :command:`export(PACKAGE)` command now does nothing unless
  enabled via :variable:`CMAKE_EXPORT_PACKAGE_REGISTRY`.
  See policy :policy:`CMP0090`.
