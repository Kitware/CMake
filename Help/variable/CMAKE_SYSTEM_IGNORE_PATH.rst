CMAKE_SYSTEM_IGNORE_PATH
------------------------

Path to be ignored by FIND_XXX() commands.

Specifies directories to be ignored by searches in FIND_XXX()
commands.  This is useful in cross-compiled environments where some
system directories contain incompatible but possibly linkable
libraries.  For example, on cross-compiled cluster environments, this
allows a user to ignore directories containing libraries meant for the
front-end machine that modules like FindX11 (and others) would
normally search.  By default this contains a list of directories
containing incompatible binaries for the host system.  See also
CMAKE_SYSTEM_PREFIX_PATH, CMAKE_SYSTEM_LIBRARY_PATH,
CMAKE_SYSTEM_INCLUDE_PATH, and CMAKE_SYSTEM_PROGRAM_PATH.
