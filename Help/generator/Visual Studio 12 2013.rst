Visual Studio 12 2013
---------------------

Generates Visual Studio 12 (VS 2013) project files.

It is possible to append a space followed by the platform name to
create project files for a specific target platform.  E.g.
"Visual Studio 12 2013 Win64" will create project files for the
x64 processor; "Visual Studio 12 2013 ARM" for ARM.

For compatibility with CMake versions prior to 3.0, one may specify this
generator using the name "Visual Studio 12" without the year component.
