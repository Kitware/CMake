subdirectory-sources
--------------------

* The :command:`target_sources` command now interprets relative source file
  paths as relative to the current source directory.  This simplifies
  incrementally building up a target's sources from subdirectories.  The
  :policy:`CMP0076` policy was added to provide backward compatibility with
  the old behavior where required.
