per-lang-link-library-flag
--------------------------

* New variables :variable:`CMAKE_<LANG>_LINK_LIBRARY_FLAG`,
  :variable:`CMAKE_<LANG>_LINK_LIBRARY_FILE_FLAG`, and
  :variable:`CMAKE_<LANG>_LINK_LIBRARY_SUFFIX` allow control of the
  flag used to specify linking to a library on a per-language basis.
  This is useful for mixed-language projects where the different
  drivers may use different flags.
