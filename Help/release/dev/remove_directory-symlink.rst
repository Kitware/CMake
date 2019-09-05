remove_directory-symlink
------------------------

* The :manual:`cmake(1)` ``-E remove_directory`` command-line tool,
  when given the path to a symlink to a directory, now removes just
  the symlink.  It no longer removes content of the linked directory.
