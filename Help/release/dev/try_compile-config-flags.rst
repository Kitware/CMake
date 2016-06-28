try_compile-config-flags
------------------------

* The :command:`try_compile` command source file signature now honors
  configuration-specific flags (e.g. :variable:`CMAKE_<LANG>_FLAGS_DEBUG`)
  in the generated test project.  Previously only the default such flags
  for the current toolchain were used.  See policy :policy:`CMP0066`.
