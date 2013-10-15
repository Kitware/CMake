Sublime Text 2 - Unix Makefiles
-------------------------------

Generates Sublime Text 2 project files.

Project files for Sublime Text 2 will be created in the top directory
and in every subdirectory which features a CMakeLists.txt file
containing a PROJECT() call.  Additionally Makefiles (or build.ninja
files) are generated into the build tree.  The appropriate make
program can build the project through the default make target.  A
"make install" target is also provided.
