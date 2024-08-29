cmake-pkg-config
----------------

* The :command:`cmake_pkg_config` command was added as an endpoint for using
  CMake's native pkg-config format parser. The only supported option in this
  release is ``EXTRACT``, which provides low-level access to the values
  produced by parsing a pkg-config file. For most users, this is not yet a
  suitable replacement for the :module:`FindPkgConfig` module.
