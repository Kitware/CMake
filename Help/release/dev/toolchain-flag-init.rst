toolchain-flag-init
-------------------

* :variable:`Toolchain files <CMAKE_TOOLCHAIN_FILE>` may now set a
  :variable:`CMAKE_<LANG>_FLAGS_INIT` variable to initialize the
  :variable:`CMAKE_<LANG>_FLAGS` cache entry the first time a language is
  enabled in a build tree.

* :variable:`Toolchain files <CMAKE_TOOLCHAIN_FILE>` may now set
  :variable:`CMAKE_EXE_LINKER_FLAGS_INIT`,
  :variable:`CMAKE_SHARED_LINKER_FLAGS_INIT`, and
  :variable:`CMAKE_MODULE_LINKER_FLAGS_INIT` variables to initialize the
  :variable:`CMAKE_EXE_LINKER_FLAGS`,
  :variable:`CMAKE_SHARED_LINKER_FLAGS`, and
  :variable:`CMAKE_MODULE_LINKER_FLAGS` cache entries the first time
  a language is enabled in a build tree.
