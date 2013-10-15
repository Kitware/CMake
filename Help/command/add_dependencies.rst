add_dependencies
----------------

Add a dependency between top-level targets.

::

  add_dependencies(<target> [<target-dependency>]...)

Make a top-level <target> depend on other top-level targets to ensure
that they build before <target> does.  A top-level target is one
created by ADD_EXECUTABLE, ADD_LIBRARY, or ADD_CUSTOM_TARGET.
Dependencies added to an IMPORTED target are followed transitively in
its place since the target itself does not build.

See the DEPENDS option of ADD_CUSTOM_TARGET and ADD_CUSTOM_COMMAND for
adding file-level dependencies in custom rules.  See the
OBJECT_DEPENDS option in SET_SOURCE_FILES_PROPERTIES to add file-level
dependencies to object files.
