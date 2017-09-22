cpack-rpm-deb-version
---------------------

* Modules :module:`CPackRPM` and :module:`CPackDeb` learned to set package epoch
  version.
  See :variable:`CPACK_RPM_PACKAGE_EPOCH` and
  :variable:`CPACK_DEBIAN_PACKAGE_EPOCH` variables.

* The :module:`CPackDeb` module learned to set package release version in
  `Version` info property.
  See :variable:`CPACK_DEBIAN_PACKAGE_RELEASE` variable.

* The :module:`CPackDeb` module learned more strict package version checking
  that complies with Debian rules.
