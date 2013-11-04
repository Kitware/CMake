INCLUDE_DIRECTORIES
-------------------

List of preprocessor include file search directories.

This property specifies the list of directories given so far to the
include_directories command.  This property exists on directories and
targets.  In addition to accepting values from the :command:`include_directories`
command, values may be set directly on any directory or any target
using the :command:`set_property` command.  A target gets its initial value for
this property from the value of the directory property.  A directory
gets its initial value from its parent directory if it has one.  Both
directory and target property values are adjusted by calls to the
:command:`include_directories` command.

The target property values are used by the generators to set the
include paths for the compiler.  See also the :command:`include_directories`
and :command:`target_include_directories` commands.

Relative paths should not be added to this property directly. Use one of
the commands above instead to handle relative paths.

Contents of INCLUDE_DIRECTORIES may use "generator expressions" with the
syntax "$<...>".  See the :manual:`cmake-generator-expressions(7)` manual for
available expressions.
