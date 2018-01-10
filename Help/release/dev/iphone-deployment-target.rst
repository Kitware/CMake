iphone-deployment-target
------------------------

* The minimum deployment target set in the
  :variable:`CMAKE_OSX_DEPLOYMENT_TARGET` variable used to be only
  applied for macOS regardless of the selected SDK.  It is now properly
  set for the target platform selected by :variable:`CMAKE_OSX_SYSROOT`.

  If for example the sysroot variable specifies an iOS SDK then the
  value in ``CMAKE_OSX_DEPLOYMENT_TARGET`` is interpreted as minimum
  iOS version.
