CMAKE_GENERATOR_TOOLSET
-----------------------

Native build system toolset name specified by user.

Some CMake generators support a toolset name to be given to the native
build system to choose a compiler.  If the user specifies a toolset
name (e.g.  via the cmake -T option) the value will be available in
this variable.
