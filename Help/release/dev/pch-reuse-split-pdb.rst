pch-reuse-split-pdb
-------------------

* With MSVC and a separate program database (``/Zi`` or ``/ZI``), targets
  using :prop_tgt:`PRECOMPILE_HEADERS_REUSE_FROM` no longer lose debug
  information when the reused target changes but its precompiled header does
  not.

* With MSVC and a separate program database (``/Zi`` or ``/ZI``), targets
  using :prop_tgt:`PRECOMPILE_HEADERS_REUSE_FROM` now write the debug
  information of sources not compiled with the precompiled header, such as
  those with :prop_sf:`SKIP_PRECOMPILE_HEADERS`, to their own compiler
  ``.pdb`` file, so it survives a rebuild of the precompiled header.
  See policy :policy:`CMP0235`.
