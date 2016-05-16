cpack-rpm-adding-dist-to-release-tag
------------------------------------

* The :module:`CPackRPM` module learned how to set RPM ``dist`` tag as part of
  RPM ``Release:`` tag when enabled (mandatory on some Linux distributions for
  e.g. on Fedora).
  See :variable:`CPACK_RPM_PACKAGE_RELEASE_DIST`.
