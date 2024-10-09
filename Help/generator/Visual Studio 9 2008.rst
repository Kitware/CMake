Visual Studio 9 2008
--------------------

Removed.  This once generated Visual Studio 9 2008 project files, but
the generator has been removed since CMake 3.30.  It is still possible
to build with VS 9 2008 tools using the :generator:`Visual Studio 14 2015`
generator (or above, and with VS 10 2010 also installed) with
:variable:`CMAKE_GENERATOR_TOOLSET` set to ``v90``, or by using
the :generator:`NMake Makefiles` generator.
