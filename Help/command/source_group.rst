source_group
------------

Define a grouping for sources in the makefile.

::

  source_group(name [REGULAR_EXPRESSION regex] [FILES src1 src2 ...])

Defines a group into which sources will be placed in project files.
This is mainly used to setup file tabs in Visual Studio.  Any file
whose name is listed or matches the regular expression will be placed
in this group.  If a file matches multiple groups, the LAST group that
explicitly lists the file will be favored, if any.  If no group
explicitly lists the file, the LAST group whose regular expression
matches the file will be favored.

The name of the group may contain backslashes to specify subgroups:

::

  source_group(outer\\inner ...)

For backwards compatibility, this command also supports the format:

::

  source_group(name regex)
