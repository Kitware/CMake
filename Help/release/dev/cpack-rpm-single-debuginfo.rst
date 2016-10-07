cpack-rpm-single-debuginfo
--------------------------

* The :module:`CPackRPM` module learned to generate main component package
  which forces generation of a rpm for defined component without component
  suffix in filename and package name.
  See :variable:`CPACK_RPM_MAIN_COMPONENT` variable.

* The :module:`CPackRPM` module learned to generate a single debuginfo package
  on demand even if components packagin is used.
  See :variable:`CPACK_RPM_DEBUGINFO_SINGLE_PACKAGE` variable.
