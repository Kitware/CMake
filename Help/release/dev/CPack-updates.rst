CPack-updates
-------------

* The :module:`CPack` module no longer mangles settings with CMake-special
  characters when they're used as defaults for other settings. The macro
  ``cpack_set_if_not_set``, which was responsible for this, is now deprecated.

* The :module:`CPack` module gained a new setting, ``CPACK_VERBATIM_VARIABLES``,
  which can be used to ensure the cpack program receives the settings' values
  exactly as they were set, even if they contain CMake-special characters.
  For compatibility, it's off by default.
