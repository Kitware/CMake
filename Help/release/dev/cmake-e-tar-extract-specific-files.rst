cmake-e-tar-extract-specific-files
----------------------------------

* The :manual:`cmake(1)` ``-E tar`` tool allow for extract (``-x``) or list
  (``-t``) only specific files or directories.  To select pathnames append
  a space-separated list of file names or directories.
  When extracting selected files or directories, you must provide their exact
  pathname, as printed by list (``-t``)
