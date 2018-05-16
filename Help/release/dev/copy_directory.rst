copy_directory
--------------

* The :manual:`cmake(1)` ``-E copy_directory`` tool now fails when the
  source directory does not exist.  Previously it succeeded by creating
  an empty destination directory.
