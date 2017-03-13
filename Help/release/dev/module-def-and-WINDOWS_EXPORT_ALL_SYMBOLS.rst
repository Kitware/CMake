module-def-and-WINDOWS_EXPORT_ALL_SYMBOLS
-----------------------------------------

* The :prop_tgt:`WINDOWS_EXPORT_ALL_SYMBOLS` target property may now
  be used in combination with explicit ``.def`` files in order to
  export all symbols from the object files within a target plus
  an explicit list of symbols that the linker finds in dependencies
  (e.g. ``msvcrt.lib``).
