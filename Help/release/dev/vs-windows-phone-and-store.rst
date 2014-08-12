vs-windows-phone-and-store
--------------------------

* Generators for Visual Studio 11 (2012) and above learned to generate
  projects for Windows Phone and Windows Store.  One may set the
  :variable:`CMAKE_SYSTEM_NAME` variable to ``WindowsPhone``
  or ``WindowsStore`` on the :manual:`cmake(1)` command-line
  or in a :variable:`CMAKE_TOOLCHAIN_FILE` to activate these platforms.
  Also set :variable:`CMAKE_SYSTEM_VERSION` to ``8.0`` or ``8.1`` to
  specify the version of Windows to be targeted.
