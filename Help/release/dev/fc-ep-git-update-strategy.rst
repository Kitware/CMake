fc-ep-git-update-strategy
-------------------------

* The :command:`ExternalProject_Add` command gained a new
  ``GIT_REMOTE_UPDATE_STRATEGY`` keyword.  This can be used to specify how
  failed rebase operations during a git update should be handled.
  The ``CMAKE_EP_GIT_REMOTE_UPDATE_STRATEGY`` variable was also added as a
  global default and is honored by both the :module:`ExternalProject` and
  :module:`FetchContent` modules.
