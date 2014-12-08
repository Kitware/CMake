try_compile-link-flags
----------------------

* The :command:`try_compile` command source file signature now honors
  link flags (e.g. :variable:`CMAKE_EXE_LINKER_FLAGS`) in the generated
  test project.  See policy :policy:`CMP0056`.
