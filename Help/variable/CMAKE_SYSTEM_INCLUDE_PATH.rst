CMAKE_SYSTEM_INCLUDE_PATH
-------------------------

Path used for searching by FIND_FILE() and FIND_PATH().

Specifies a path which will be used both by FIND_FILE() and
FIND_PATH().  Both commands will check each of the contained
directories for the existence of the file which is currently searched.
By default it contains the standard directories for the current
system.  It is NOT intended to be modified by the project, use
CMAKE_INCLUDE_PATH for this.  See also CMAKE_SYSTEM_PREFIX_PATH.
