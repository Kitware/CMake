cmake_pkg_config(
  EXTRACT foo
  REQUIRED
)

cmake_pkg_config(
  EXTRACT does-not-exist
  REQUIRED
)
