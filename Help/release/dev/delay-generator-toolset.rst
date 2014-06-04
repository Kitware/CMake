delay-generator-toolset
-----------------------

* The :variable:`CMAKE_GENERATOR_TOOLSET` variable may now be
  initialized in a toolchain file specified by the
  :variable:`CMAKE_TOOLCHAIN_FILE` variable.  This is useful
  when cross-compiling with the Xcode or Visual Studio
  generators.
