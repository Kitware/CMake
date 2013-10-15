CMAKE_IGNORE_PATH
-----------------

Path to be ignored by FIND_XXX() commands.

Specifies directories to be ignored by searches in FIND_XXX()
commands.  This is useful in cross-compiled environments where some
system directories contain incompatible but possibly linkable
libraries.  For example, on cross-compiled cluster environments, this
allows a user to ignore directories containing libraries meant for the
front-end machine that modules like FindX11 (and others) would
normally search.  By default this is empty; it is intended to be set
by the project.  Note that CMAKE_IGNORE_PATH takes a list of directory
names, NOT a list of prefixes.  If you want to ignore paths under
prefixes (bin, include, lib, etc.), you'll need to specify them
explicitly.  See also CMAKE_PREFIX_PATH, CMAKE_LIBRARY_PATH,
CMAKE_INCLUDE_PATH, CMAKE_PROGRAM_PATH.
