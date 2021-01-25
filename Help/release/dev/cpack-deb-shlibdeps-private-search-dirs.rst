cpack-deb-shlibdeps-resolving-private-dependencies
--------------------------------------------------

* The :module:`CPackDeb` module learned a new
  :variable:`CPACK_DEBIAN_PACKAGE_SHLIBDEPS_PRIVATE_DIRS`
  variable to specify additional search directories for
  resolving private library dependencies when using
  ``dpkg-shlibdeps``.
