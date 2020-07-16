macOS-sdk-latest
----------------

* Building for macOS will now use the latest SDK available on the system,
  unless the user has explicitly chosen a SDK using :variable:`CMAKE_OSX_SYSROOT`.

  The deployment target or system macOS version will not affect
  the choice of SDK.

* macOS SDKs older than 10.5 are no longer supported.
