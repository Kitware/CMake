cpack-rpm-upper-cased-components
--------------------------------

* The "CPackRPM" module now supports upper cased component name
  in per component CPackRPM specific variables.
  E.g. component named ``foo`` now expects component specific
  variable to be ``CPACK_RPM_FOO_PACKAGE_NAME`` while before
  it expected ``CPACK_RPM_foo_PACKAGE_NAME``.
  Upper cased component name part in variables is compatible
  with convention used for other CPack variables.
  For back compatibility old format of variables is still valid
  and prefered if both versions of variable are set, but the
  preferred future use is upper cased component names in variables.
  New variables that will be added to CPackRPM in later versions
  will only support upper cased component variable format.
