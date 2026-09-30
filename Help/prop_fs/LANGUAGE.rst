LANGUAGE
--------

.. versionadded:: 4.5

Specify the language used to compile members of a ``SOURCES`` file set.

This property selects the language whose compiler and compilation rules
are used for members of the file set when building a target.  It takes
precedence over both the :prop_sf:`LANGUAGE` source file property and
language selection based on the file extension, without modifying the
source file property.

The selection applies to targets that compile sources from the file set,
including targets that inherit those sources through linked target
interfaces.  This allows the same source file to be compiled using
different languages' compilers and compilation rules in different targets.

The value must be a literal language name.  Generator expressions and
appending values are not supported.  The named language must be enabled by a
:command:`project` or :command:`enable_language` command.
