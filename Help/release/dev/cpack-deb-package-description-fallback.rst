cpack-deb-package-description-fallback
--------------------------------------

* The :module:`CPackDeb` module gained a new
  :variable:`CPACK_DEBIAN_<COMPONENT>_PACKAGE_DESCRIPTION`
  variable for component-specific package descriptions.

* The :module:`CPackDeb` module changed its package description
  override rules to match :module:`CPackRPM` module behavior.
  If the :variable:`CPACK_PACKAGE_DESCRIPTION_FILE` variable is set to
  a non-default location then it is preferred to the
  :variable:`CPACK_PACKAGE_DESCRIPTION_SUMMARY` variable.
  This is a behavior change from previous versions but produces
  more consistent and expected behavior.
  See :variable:`CPACK_DEBIAN_PACKAGE_DESCRIPTION`.
