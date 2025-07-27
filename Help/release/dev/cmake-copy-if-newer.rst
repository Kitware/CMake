cmake-copy-if-newer
-------------------

* The :manual:`cmake(1)` command-line tool now supports
  ``cmake -E copy_if_newer`` and ``cmake -E copy_directory_if_newer``
  subcommands to copy files based on timestamp comparison instead of
  content comparison. These commands copy files only if the source is
  newer than the destination, providing better performance for build
  systems compared to ``copy_if_different`` which compares file contents.
