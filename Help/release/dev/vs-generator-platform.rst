vs-generator-platform
---------------------

* The Visual Studio generators for versions 8 (2005) and above
  learned to read the target platform name from a new
  :variable:`CMAKE_GENERATOR_PLATFORM` variable when it is
  not specified as part of the generator name.  The platform
  name may be specified on the :manual:`cmake(1)` command line
  with the ``-A`` option, e.g. ``-G "Visual Studio 12 2013" -A x64``.
