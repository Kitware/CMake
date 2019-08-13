per-lang-link-library-flag
--------------------------

* The new :variable:`CMAKE_<LANG>_LINK_LIBRARY_FLAG` flag allows you to now
  control the flag used to specify linking to a library on a per-language basis.
  This is useful for mixed-language projects where the different drivers may use
  different flags.
