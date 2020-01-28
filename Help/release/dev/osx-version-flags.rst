add_osx_compatiblity_property
-----------------------------

* Target properties :prop_tgt:`OSX_COMPATIBILITY_VERSION` and
  :prop_tgt:`OSX_CURRENT_VERSION` were added to set the
  ``compatibility_version`` and ``curent_version`` respectively
  on macOS. For backwards compatibility, if these properties
  are not set, :prop_tgt:`SOVERSION` and :prop_tgt:`VERSION`
  are used respectively as fallbacks.
