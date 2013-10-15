Eclipse CDT4 - Unix Makefiles
-----------------------------

Generates Eclipse CDT 4.0 project files.

Project files for Eclipse will be created in the top directory.  In
out of source builds, a linked resource to the top level source
directory will be created.  Additionally a hierarchy of makefiles is
generated into the build tree.  The appropriate make program can build
the project through the default make target.  A "make install" target
is also provided.
