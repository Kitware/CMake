CMAKE_PREFIX_PATH
-----------------

Path used for searching by FIND_XXX(), with appropriate suffixes added.

Specifies a path which will be used by the FIND_XXX() commands.  It
contains the "base" directories, the FIND_XXX() commands append
appropriate subdirectories to the base directories.  So FIND_PROGRAM()
adds /bin to each of the directories in the path, FIND_LIBRARY()
appends /lib to each of the directories, and FIND_PATH() and
FIND_FILE() append /include .  By default it is empty, it is intended
to be set by the project.  See also CMAKE_SYSTEM_PREFIX_PATH,
CMAKE_INCLUDE_PATH, CMAKE_LIBRARY_PATH, CMAKE_PROGRAM_PATH.
