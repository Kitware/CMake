android-info-variables
----------------------

* When :ref:`Cross Compiling for Android`, a new
  :variable:`CMAKE_<LANG>_ANDROID_TOOLCHAIN_MACHINE` variable is
  now set to indicate the binutils' machine name.

* When :ref:`Cross Compiling for Android with the NDK`, the
  :variable:`CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION` variable is
  now set by CMake if it is not set by the user or toolchain file.

* When :ref:`Cross Compiling for Android with the NDK`, a new
  :variable:`CMAKE_ANDROID_NDK_TOOLCHAIN_HOST_TAG` variable is
  now set by CMake to expose the host directory component of the
  path to the NDK prebuilt toolchains.
