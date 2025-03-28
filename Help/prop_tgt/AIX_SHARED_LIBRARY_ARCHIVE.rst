AIX_SHARED_LIBRARY_ARCHIVE
--------------------------

.. versionadded:: 3.31

On AIX, enable creation of a shared library archive.  This places
the shared object ``.so`` file inside an archive ``.a`` file.

By default, CMake creates shared libraries on AIX as plain
shared object ``.so`` files for consistency with other UNIX platforms.
Alternatively, set this property to a true value to create a shared
library archive instead, as is AIX convention.

The shared object name in the archive encodes version information from
the :prop_tgt:`SOVERSION` target property, if set, and otherwise from
the :prop_tgt:`VERSION` target property, if set.

This property defaults to :variable:`CMAKE_AIX_SHARED_LIBRARY_ARCHIVE`
if that variable is set when a non-imported ``SHARED`` library target
is created by :command:`add_library`.  Imported targets must explicitly
enable :prop_tgt:`!AIX_SHARED_LIBRARY_ARCHIVE` if they import an AIX
shared library archive.
