cpack-deb-component-dependencies
--------------------------------

* The :module:`CPackDeb` module learned to set package dependencies
  per component. See :variable:`CPACK_DEBIAN_<COMPONENT>_PACKAGE_PREDEPENDS`,
  :variable:`CPACK_DEBIAN_<COMPONENT>_PACKAGE_ENHANCES`,
  :variable:`CPACK_DEBIAN_<COMPONENT>_PACKAGE_BREAKS`,
  :variable:`CPACK_DEBIAN_<COMPONENT>_PACKAGE_CONFLICTS`,
  :variable:`CPACK_DEBIAN_<COMPONENT>_PACKAGE_PROVIDES`,
  :variable:`CPACK_DEBIAN_<COMPONENT>_PACKAGE_REPLACES`,
  :variable:`CPACK_DEBIAN_<COMPONENT>_PACKAGE_RECOMMENDS` and
  :variable:`CPACK_DEBIAN_<COMPONENT>_PACKAGE_SUGGESTS`.
