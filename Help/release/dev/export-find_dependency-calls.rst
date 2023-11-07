export-find_dependency-calls
----------------------------

* :command:`install(EXPORT)` and :command:`export(EXPORT)` learned a new
  ``EXPORT_PACKAGE_DEPENDENCIES`` argument, which can be used to generate
  :command:`find_dependency` calls based on what targets the exported targets
  depend on.
* A new :command:`export(SETUP)` signature was created to configure export
  sets. This can be used to configure how :command:`find_dependency` calls are
  exported.
* A new :prop_tgt:`EXPORT_FIND_PACKAGE_NAME` target property was created to
  allow targets to specify what package name to pass when exporting
  :command:`find_dependency` calls. This property is initialized with a new
  :variable:`CMAKE_EXPORT_FIND_PACKAGE_NAME` variable.
* :command:`FetchContent_MakeAvailable` now sets the
  :variable:`CMAKE_EXPORT_FIND_PACKAGE_NAME` variable for CMake projects.
