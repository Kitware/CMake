CMAKE_BACKWARDS_COMPATIBILITY
-----------------------------

Version of cmake required to build project

From the point of view of backwards compatibility, this specifies what
version of CMake should be supported.  By default this value is the
version number of CMake that you are running.  You can set this to an
older version of CMake to support deprecated commands of CMake in
projects that were written to use older versions of CMake.  This can
be set by the user or set at the beginning of a CMakeLists file.
