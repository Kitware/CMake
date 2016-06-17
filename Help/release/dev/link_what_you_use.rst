link_what_you_use
-----------------

* A :prop_tgt:`LINK_WHAT_YOU_USE` target property and supporting
  :variable:`CMAKE_LINK_WHAT_YOU_USE` variable were introduced
  to detect (on UNIX) shared libraries that are linked but not
  needed by running ``ldd -r -u``.
