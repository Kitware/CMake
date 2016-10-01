doc-compilers-dropped
---------------------

* Support for building CMake itself with some compilers was dropped:

  * Visual Studio 7.1 and 2005 -- superseded by VS 2008 and above
  * MinGW.org mingw32 -- superseded by MSYS2 mingw32 and mingw64

  CMake still supports generating build systems for other projects using
  these compilers.
