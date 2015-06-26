auto_export_dll_symbols
-----------------------

* On Windows with MS-compatible tools, CMake learned to optionally
  generate a module definition (``.def``) file for ``SHARED`` libraries.
  See the :prop_tgt:`WINDOWS_EXPORT_ALL_SYMBOLS` target property.
