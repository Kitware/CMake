system
------

* The :prop_tgt:`SYSTEM` target property was added to specify
  that a target should be treated as a system library (i.e.
  its include directories are automatically ``SYSTEM`` when
  compiling consumers).

* The :prop_tgt:`EXPORT_NO_SYSTEM` target property was added to
  specify that :command:`install(EXPORT)` and :command:`export`
  commands will generate a imported target with
  :prop_tgt:`SYSTEM` property `OFF`.

* The :prop_tgt:`IMPORTED_NO_SYSTEM` target property was deprecated
  in favor of :prop_tgt:`SYSTEM` and :prop_tgt:`EXPORT_NO_SYSTEM`.
