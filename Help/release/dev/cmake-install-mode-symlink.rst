cmake-install-mode-symlink
--------------------------

* The :envvar:`CMAKE_INSTALL_MODE` environment variable was added to
  allow users to override the default file-copying behavior of
  :command:`install` and :command:`file(INSTALL)` into creating
  symbolic links. This can aid in lowering storage space requirements
  and avoiding redundancy.

* The :command:`file(INSTALL)` can now be affected / modified by the
  :envvar:`CMAKE_INSTALL_MODE` environment variable causing installation
  of symbolic links instead of copying of files.

* The :command:`install` can now be affected / modified by the
  :envvar:`CMAKE_INSTALL_MODE` environment variable causing installation
  of symbolic links instead of copying of files.
