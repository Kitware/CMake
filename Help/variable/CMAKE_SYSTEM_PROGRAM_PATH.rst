CMAKE_SYSTEM_PROGRAM_PATH
-------------------------

Path used for searching by FIND_PROGRAM().

Specifies a path which will be used by FIND_PROGRAM().  FIND_PROGRAM()
will check each of the contained directories for the existence of the
program which is currently searched.  By default it contains the
standard directories for the current system.  It is NOT intended to be
modified by the project, use CMAKE_PROGRAM_PATH for this.  See also
CMAKE_SYSTEM_PREFIX_PATH.
