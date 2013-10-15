MACOSX_RPATH
------------

Whether to use rpaths on Mac OS X.

When this property is set to true, the directory portion of
the"install_name" field of shared libraries will default to
"@rpath".Runtime paths will also be embedded in binaries using this
target.This property is initialized by the value of the variable
CMAKE_MACOSX_RPATH if it is set when a target is created.
