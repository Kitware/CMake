CMAKE_PROGRAM_PATH
------------------

Path used for searching by FIND_PROGRAM().

Specifies a path which will be used by FIND_PROGRAM().  FIND_PROGRAM()
will check each of the contained directories for the existence of the
program which is currently searched.  By default it is empty, it is
intended to be set by the project.  See also
CMAKE_SYSTEM_PROGRAM_PATH, CMAKE_PREFIX_PATH.
