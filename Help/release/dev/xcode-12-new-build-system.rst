xcode-12-new-build-system
-------------------------

* The :generator:`Xcode` generator now uses the Xcode "new build system"
  when generating for Xcode 12.0 or higher.
  See the :variable:`CMAKE_XCODE_BUILD_SYSTEM` variable.
  One may use ``-T buildsystem=1`` to switch to the legacy build system.
