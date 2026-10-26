try_compile-module-base-dirs
----------------------------

* The :command:`try_compile` and :command:`try_run` commands now accept
  ``CXX_MODULE`` sources outside of the calling project's source directory,
  including copies made by ``SOURCE_FROM_CONTENT``, ``SOURCE_FROM_FILE``
  and ``SOURCE_FROM_VAR``.  Previously such sources caused an error.
