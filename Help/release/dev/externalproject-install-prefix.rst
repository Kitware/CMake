externalproject-install-prefix
------------------------------

* The :command:`ExternalProject_Add` command now sets
  :variable:`CMAKE_INSTALL_PREFIX` to the external project's ``INSTALL_DIR`` in
  its default configure command, so a CMake external project installs into the
  prefix the module reserves for it rather than the built-in default (e.g.
  ``/usr/local``). A caller-supplied prefix still takes precedence. See policy
  :policy:`CMP0225`.
