CMAKE_SYSTEM_LIBRARY_PATH
-------------------------

Path used for searching by FIND_LIBRARY().

Specifies a path which will be used by FIND_LIBRARY().  FIND_LIBRARY()
will check each of the contained directories for the existence of the
library which is currently searched.  By default it contains the
standard directories for the current system.  It is NOT intended to be
modified by the project, use CMAKE_LIBRARY_PATH for this.  See also
CMAKE_SYSTEM_PREFIX_PATH.
