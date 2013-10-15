CMAKE_BUILD_TOOL
----------------

Tool used for the actual build process.

This variable is set to the program that will be needed to build the
output of CMake.  If the generator selected was Visual Studio 6, the
CMAKE_BUILD_TOOL will be set to msdev, for Unix Makefiles it will be
set to make or gmake, and for Visual Studio 7 it set to devenv.  For
NMake Makefiles the value is nmake.  This can be useful for adding
special flags and commands based on the final build environment.
