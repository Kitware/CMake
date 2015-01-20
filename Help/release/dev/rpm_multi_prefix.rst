rpm_multi_prefix
----------------

* The :module:`CPackRPM` module learned a new
  :variable:`CPACK_RPM_<COMPONENT>_PACKAGE_PREFIX` variable to
  specify a component-specific value to use instead of
  :variable:`CPACK_PACKAGING_INSTALL_PREFIX`.

* The :module:`CPackRPM` module learned a new
  :variable:`CPACK_RPM_RELOCATION_PATHS` variable to
  specify multiple relocation prefixes for a single rpm package.
