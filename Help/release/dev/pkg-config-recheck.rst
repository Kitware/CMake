pkg-config-recheck
------------------

* Calls to the :module:`FindPkgConfig` module :command:`pkg_check_modules`
  command following a successful call learned to re-evaluate the cached values
  for a given prefix after changes to the parameters to the command for that
  prefix.
