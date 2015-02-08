Qbs
---

Generates Qbs project files.

Project files for Qbs will be created in the top directory and
in every subdirectory which features a CMakeLists.txt file containing
a PROJECT() call.  Additionally a hierarchy of makefiles is generated
into the build tree.  The appropriate make program can build the
project through the default make target.  A "make install" target is
also provided.

This "extra" generator may be specified as:

``Qbs - MinGW Makefiles``
 Generate with :generator:`MinGW Makefiles`.

``Qbs - NMake Makefiles``
 Generate with :generator:`NMake Makefiles`.

``Qbs - Ninja``
 Generate with :generator:`Ninja`.

``Qbs - Unix Makefiles``
 Generate with :generator:`Unix Makefiles`.
