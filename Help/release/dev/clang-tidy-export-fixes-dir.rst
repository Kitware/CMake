clang-tidy-export-fixes-dir
---------------------------

* A new :prop_tgt:`<LANG>_CLANG_TIDY_EXPORT_FIXES_DIR` target property was
  created to allow the ``clang-tidy`` tool to export its suggested fixes to a
  set of ``.yaml`` files. A new
  :variable:`CMAKE_<LANG>_CLANG_TIDY_EXPORT_FIXES_DIR` variable was created to
  initialize this property.
