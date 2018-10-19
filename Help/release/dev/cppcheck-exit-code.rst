cppcheck-exit-code
------------------

* When using cppcheck via the :variable:`CMAKE_<LANG>_CPPCHECK` variable
  or :prop_tgt:`<LANG>_CPPCHECK` property, the build will now fail if
  ``cppcheck`` returns non-zero as configured by its command-line options.
