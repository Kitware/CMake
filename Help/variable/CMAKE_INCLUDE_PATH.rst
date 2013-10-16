CMAKE_INCLUDE_PATH
------------------

Path used for searching by FIND_FILE() and FIND_PATH().

Specifies a path which will be used both by FIND_FILE() and
FIND_PATH().  Both commands will check each of the contained
directories for the existence of the file which is currently searched.
By default it is empty, it is intended to be set by the project.  See
also CMAKE_SYSTEM_INCLUDE_PATH, CMAKE_PREFIX_PATH.
