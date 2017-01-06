capture-clang-tidy-errors
-------------------------

* If a command specified by the :prop_tgt:`<LANG>_CLANG_TIDY` target property
  returns non-zero at build time this is now treated as an error instead of
  silently ignored.
