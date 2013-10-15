set_directory_properties
------------------------

Set a property of the directory.

::

  set_directory_properties(PROPERTIES prop1 value1 prop2 value2)

Set a property for the current directory and subdirectories.  If the
property is not found, CMake will report an error.  The properties
include: INCLUDE_DIRECTORIES, LINK_DIRECTORIES,
INCLUDE_REGULAR_EXPRESSION, and ADDITIONAL_MAKE_CLEAN_FILES.
ADDITIONAL_MAKE_CLEAN_FILES is a list of files that will be cleaned as
a part of "make clean" stage.
