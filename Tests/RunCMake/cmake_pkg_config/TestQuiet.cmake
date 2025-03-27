cmake_pkg_config(
  EXTRACT foo
  QUIET
  STRICTNESS STRICT
)

cmake_pkg_config(
  EXTRACT no-name
  QUIET
  STRICTNESS STRICT
)

cmake_pkg_config(
  EXTRACT empty-key
  QUIET
  STRICTNESS STRICT
)

cmake_pkg_config(
  EXTRACT cflags-bothcase-f
  QUIET
  STRICTNESS STRICT
)

cmake_pkg_config(
  EXTRACT does-not-exist
  QUIET
  STRICTNESS STRICT
)
