android-ndk
-----------

* CMake's support for :ref:`Cross Compiling for Android`
  is now merged with the Android NDK's toolchain file.
  They now have similar behavior, though some variable names differ.
  User-facing changes include:

  - ``find_*`` functions will search NDK ABI / API specific paths by default.

  - The default :variable:`CMAKE_BUILD_TYPE` for Android is
    now ``RelWithDebInfo``.
