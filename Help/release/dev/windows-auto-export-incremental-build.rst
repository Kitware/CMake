windows-auto-export-incremental-build
-------------------------------------

* On Windows, existing auto generated exports are now only updated if the
  modified time stamp of the exports is not newer than any modified time stamp
  of the input files.
