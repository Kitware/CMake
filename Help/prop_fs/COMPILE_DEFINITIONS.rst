COMPILE_DEFINITIONS
-------------------

.. versionadded:: 4.4

Preprocessor definitions for compiling a source file.

The ``COMPILE_DEFINITIONS`` property may be set to a semicolon-separated
list of preprocessor definitions using the syntax ``VAR`` or ``VAR=value``.
Function-style definitions are not supported.  CMake will
automatically escape the value correctly for the native build system
(note that CMake language syntax may require escapes to specify some
values).

CMake will automatically drop some definitions that are not supported
by the native build tool.  :generator:`Xcode` does not support
per-configuration definitions on source files.

.. include:: /include/COMPILE_DEFINITIONS_DISCLAIMER.rst

Contents of ``COMPILE_DEFINITIONS`` may use :manual:`cmake-generator-expressions(7)`
with the syntax ``$<...>``.  See the :manual:`cmake-generator-expressions(7)`
manual for available expressions.  However, :generator:`Xcode`
does not support per-config per-source settings, so expressions
that depend on the build configuration are not allowed with that
generator.

Related properties:

* Use :prop_fs:`COMPILE_OPTIONS` to pass additional compile flags.
* Use :prop_fs:`INCLUDE_DIRECTORIES` to pass additional include directories.

Related commands:

* :command:`add_compile_definitions` for directory-wide settings
* :command:`target_compile_definitions` for target-specific settings
* :command:`set_source_files_properties` for source-specific settings
