cpack-deb-autodep-ORIGIN-RPATH
--------------------------------

* The "CPackDeb" module learned how to handle ``$ORIGIN``
  in ``CMAKE_INSTALL_RPATH`` when :variable:`CPACK_DEBIAN_PACKAGE_SHLIBDEPS`
  is used for dependency auto detection.
