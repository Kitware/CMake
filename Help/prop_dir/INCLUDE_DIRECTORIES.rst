INCLUDE_DIRECTORIES
-------------------

List of preprocessor include file search directories.

This property specifies the list of directories given so far to the
include_directories command.  This property exists on directories and
targets.  In addition to accepting values from the include_directories
command, values may be set directly on any directory or any target
using the set_property command.  A target gets its initial value for
this property from the value of the directory property.  A directory
gets its initial value from its parent directory if it has one.  Both
directory and target property values are adjusted by calls to the
include_directories command.

The target property values are used by the generators to set the
include paths for the compiler.  See also the include_directories
command.
