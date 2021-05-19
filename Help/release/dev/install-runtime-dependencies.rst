install-runtime-dependencies
----------------------------

* The :command:`install(TARGETS)` command gained new ``RUNTIME_DEPENDENCIES``
  and ``RUNTIME_DEPENDENCY_SET`` arguments, which can be used to install
  runtime dependencies using :command:`file(GET_RUNTIME_DEPENDENCIES)`.
* The :command:`install` command gained a new ``RUNTIME_DEPENDENCY_SET`` mode,
  which can be used to install runtime dependencies using
  :command:`file(GET_RUNTIME_DEPENDENCIES)`.
