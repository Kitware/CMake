add_androidmk_generator
-----------------------

* The :command:`install` command gained an ``EXPORT_ANDROID_MK``
  subcommand to install ``Android.mk`` files referencing installed
  libraries as prebuilts for the Android NDK build system.

* The :command:`export` command gained an ``ANDROID_MK`` option
  to generate ``Android.mk`` files referencing CMake-built
  libraries as prebuilts for the Android NDK build system.
