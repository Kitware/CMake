CMAKE_ENABLE_EXPORTS
--------------------

.. versionadded:: 3.4

Specify whether executables export symbols for loadable modules.

This variable is used to initialize the :prop_tgt:`ENABLE_EXPORTS` target
property for executable targets when they are created by calls to the
:command:`add_executable` command.  See the property documentation for details.

This command has been superseded by the
:variable:`CMAKE_EXECUTABLE_ENABLE_EXPORTS` command.  It is provided for
compatibility with older CMake code.
