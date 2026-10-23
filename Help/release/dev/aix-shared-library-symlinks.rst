aix-shared-library-symlinks
---------------------------

* On AIX, the :prop_tgt:`AIX_SHARED_LIBRARY_SYMLINKS` target property and
  :variable:`CMAKE_AIX_SHARED_LIBRARY_SYMLINKS` variable were added to name
  plain shared objects with :prop_tgt:`VERSION` and :prop_tgt:`SOVERSION`
  and create symlinks.  See policy :policy:`CMP0227`.
