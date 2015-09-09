CPack-updates
-------------

* The :module:`CPack` module no longer mangles settings with CMake-special
  characters when they're used as defaults for other settings. The macro
  ``cpack_set_if_not_set``, which was responsible for this, is now deprecated.
