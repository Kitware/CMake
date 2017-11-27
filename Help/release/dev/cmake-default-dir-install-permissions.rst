cmake-default-dir-install-permissions
-------------------------------------

* The :variable:`CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS` variable was added
  to enable setting of default permissions for directories created implicitly
  during installation of files by :command:`install` and
  :command:`file(INSTALL)`.

* The :variable:`CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS` variable was added
  which serves the same purpose during packaging as the
  :variable:`CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS` variable serves during
  installation (e.g. ``make install``).
