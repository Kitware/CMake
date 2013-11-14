CMAKE_SYSTEM_PREFIX_PATH
------------------------

Path used for searching by FIND_XXX(), with appropriate suffixes added.

Specifies a path which will be used by the FIND_XXX() commands.  It
contains the "base" directories, the FIND_XXX() commands append
appropriate subdirectories to the base directories.  So FIND_PROGRAM()
adds /bin to each of the directories in the path, FIND_LIBRARY()
appends /lib to each of the directories, and FIND_PATH() and
FIND_FILE() append /include .  By default this contains the standard
directories for the current system, the CMAKE_INSTALL_PREFIX and
the :variable:`CMAKE_STAGING_PREFIX`.  It is NOT intended to be modified by
the project, use CMAKE_PREFIX_PATH for this.  See also CMAKE_SYSTEM_INCLUDE_PATH,
CMAKE_SYSTEM_LIBRARY_PATH, CMAKE_SYSTEM_PROGRAM_PATH, and
CMAKE_SYSTEM_IGNORE_PATH.
