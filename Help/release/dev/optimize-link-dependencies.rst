optimize-link-dependencies
--------------------------

* A new target property, :prop_tgt:`OPTIMIZE_DEPENDENCIES`, was added to
  avoid unnecessarily building dependencies for a static library.
* A new variable, :variable:`CMAKE_OPTIMIZE_DEPENDENCIES`, was added to
  initialize the :prop_tgt:`OPTIMIZE_DEPENDENCIES` target property.
