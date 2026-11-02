embtclang-toolchain
-------------------

* Embarcadero Clang-based compilers are now supported with
  :variable:`compiler id <CMAKE_<LANG>_COMPILER_ID>` ``EMBTClang``.
  C++ modules are not yet supported with this compiler.

* The runtime library for the Embarcadero ``bcc64x`` toolchain may now be
  selected with the :prop_tgt:`EMBT_RUNTIME_LIBRARY` target property.
