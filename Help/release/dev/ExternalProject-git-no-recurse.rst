ExternalProject-git-no-recurse
------------------------------

* The :module:`ExternalProject` module :command:`ExternalProject_Add`
  command gained a ``GIT_SUBMODULES_RECURSE`` option to specify whether
  Git submodules should be updated recursively.  The default is on to
  preserve existing behavior.
