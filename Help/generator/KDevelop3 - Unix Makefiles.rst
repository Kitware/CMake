KDevelop3 - Unix Makefiles
--------------------------

Generates KDevelop 3 project files.

Project files for KDevelop 3 will be created in the top directory and
in every subdirectory which features a CMakeLists.txt file containing
a PROJECT() call.  If you change the settings using KDevelop cmake
will try its best to keep your changes when regenerating the project
files.  Additionally a hierarchy of UNIX makefiles is generated into
the build tree.  Any standard UNIX-style make program can build the
project through the default make target.  A "make install" target is
also provided.
