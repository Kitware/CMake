Ninja
-----

Generates build.ninja files.

A build.ninja file is generated into the build tree.  Recent versions
of the ninja program can build the project through the "all" target.
An "install" target is also provided.

For each subdirectory ``sub/dir`` of the project an additional target
named ``sub/dir/all`` is generated that depends on all targets required
by that subdirectory.
