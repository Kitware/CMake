generator-instance
------------------

* A :variable:`CMAKE_GENERATOR_INSTANCE` variable was introduced
  to hold the selected instance of the generator's corresponding
  native tools if multiple are available.  This is used by the
  :generator:`Visual Studio 15 2017` generator to hold the
  selected instance of Visual Studio persistently.
