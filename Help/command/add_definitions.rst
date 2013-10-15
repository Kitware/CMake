add_definitions
---------------

Adds -D define flags to the compilation of source files.

::

  add_definitions(-DFOO -DBAR ...)

Adds flags to the compiler command line for sources in the current
directory and below.  This command can be used to add any flags, but
it was originally intended to add preprocessor definitions.  Flags
beginning in -D or /D that look like preprocessor definitions are
automatically added to the COMPILE_DEFINITIONS property for the
current directory.  Definitions with non-trivial values may be left in
the set of flags instead of being converted for reasons of backwards
compatibility.  See documentation of the directory, target, and source
file COMPILE_DEFINITIONS properties for details on adding preprocessor
definitions to specific scopes and configurations.
