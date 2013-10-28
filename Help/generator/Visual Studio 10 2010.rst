Visual Studio 10 2010
---------------------

Generates Visual Studio 10 (VS 2010) project files.

It is possible to append a space followed by the platform name to
create project files for a specific target platform.  E.g.
"Visual Studio 10 2010 Win64" will create project files for the
x64 processor; "Visual Studio 10 2010 IA64" for Itanium.

For compatibility with CMake versions prior to 3.0, one may specify this
generator using the name "Visual Studio 10" without the year component.
