Visual Studio 10 2010
---------------------

Removed.  This once generated Visual Studio 10 2010 project files, but
the generator has been removed since CMake 3.25.  It is still possible
to build with VS 10 2010 tools using the :generator:`Visual Studio 14 2015`
(or above) generator with :variable:`CMAKE_GENERATOR_TOOLSET` set to
``v100``, or by using the :generator:`NMake Makefiles` generator.
