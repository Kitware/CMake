vs-host-x64-tools
-----------------

* The :ref:`Visual Studio Generators` for VS 2013 and above learned to
  support a ``host=x64`` option in the :variable:`CMAKE_GENERATOR_TOOLSET`
  value (e.g.  via the :manual:`cmake(1)` ``-T`` option) to request use
  of a VS 64-bit toolchain on 64-bit hosts.
