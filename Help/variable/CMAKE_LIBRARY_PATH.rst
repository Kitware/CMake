CMAKE_LIBRARY_PATH
------------------

Path used for searching by FIND_LIBRARY().

Specifies a path which will be used by FIND_LIBRARY().  FIND_LIBRARY()
will check each of the contained directories for the existence of the
library which is currently searched.  By default it is empty, it is
intended to be set by the project.  See also
CMAKE_SYSTEM_LIBRARY_PATH, CMAKE_PREFIX_PATH.
