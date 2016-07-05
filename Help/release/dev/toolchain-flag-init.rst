toolchain-flag-init
-------------------

* :variable:`Toolchain files <CMAKE_TOOLCHAIN_FILE>` may now set a
  :variable:`CMAKE_<LANG>_FLAGS_INIT` variable to initialize the
  :variable:`CMAKE_<LANG>_FLAGS` cache entry the first time a language is
  enabled in a build tree.
