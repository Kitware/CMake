try_compile-target-type
-----------------------

* The :command:`try_compile` command learned to check a new
  :variable:`CMAKE_TRY_COMPILE_TARGET_TYPE` variable to optionally
  build a static library instead of an executable.  This is useful
  for cross-compiling toolchains that cannot link binaries without
  custom flags or scripts.
