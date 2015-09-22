restrict-shlib-link-flags-to-enable-exports
-------------------------------------------

* CMake no longer links executables with flags to export symbols
  unless the :prop_tgt:`ENABLE_EXPORTS` target property is set.
  See policy :policy:`CMP0065`.
