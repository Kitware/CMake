COMPILE_DEFINITIONS
-------------------

.. versionadded:: 4.4

Preprocessor definitions for compiling a source file.

The ``COMPILE_DEFINITIONS`` property may be set to a :ref:`semicolon-separated
list <CMake Language Lists>` of preprocessor definitions using the syntax
``VAR`` or ``VAR=value``. Function-style definitions are not supported.  CMake
will automatically escape the value correctly for the native build system (note
that CMake language syntax may require escapes to specify some values).

CMake will automatically drop some definitions that are not supported
by the native build tool.  :generator:`Xcode` does not support
per-configuration definitions on source files.

.. include:: /include/COMPILE_DEFINITIONS_DISCLAIMER.rst

.. |genex_subject| replace:: ``COMPILE_DEFINITIONS``
.. include:: ../include/GENEX_NOTE_PER_SOURCE.rst

Related properties:

* Use :prop_fs:`COMPILE_OPTIONS` to pass additional compile flags.
* Use :prop_fs:`INCLUDE_DIRECTORIES` to pass additional include directories.

Related commands:

* :command:`add_compile_definitions` for directory-wide settings
* :command:`target_compile_definitions` for target-specific settings
* :command:`set_source_files_properties` for source-specific settings
