CMAKE_AIX_SHARED_LIBRARY_SYMLINKS
---------------------------------

.. versionadded:: 4.6

On AIX, enable or disable :prop_tgt:`VERSION` and :prop_tgt:`SOVERSION`
file names for plain shared objects.

This variable initializes the :prop_tgt:`AIX_SHARED_LIBRARY_SYMLINKS`
target property on non-imported ``SHARED`` library targets as they are
created by :command:`add_library`.  See that target property for details.
