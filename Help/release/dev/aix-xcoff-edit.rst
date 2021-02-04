aix-xcoff-edit
--------------

* On AIX, installation of XCOFF executables and shared libraries
  no longer requires relinking to change the runtime search path
  from the build-tree RPATH to the install-tree RPATH.  CMake now
  edits the XCOFF binaries directly during installation, as has
  long been done on ELF platforms.
