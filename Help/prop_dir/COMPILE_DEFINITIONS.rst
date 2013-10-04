COMPILE_DEFINITIONS
-------------------

Preprocessor definitions for compiling a directory's sources.

The COMPILE_DEFINITIONS property may be set to a semicolon-separated
list of preprocessor definitions using the syntax VAR or VAR=value.
Function-style definitions are not supported.  CMake will
automatically escape the value correctly for the native build system
(note that CMake language syntax may require escapes to specify some
values).  This property may be set on a per-configuration basis using
the name COMPILE_DEFINITIONS_<CONFIG> where <CONFIG> is an upper-case
name (ex.  "COMPILE_DEFINITIONS_DEBUG").  This property will be
initialized in each directory by its value in the directory's parent.

CMake will automatically drop some definitions that are not supported
by the native build tool.  The VS6 IDE does not support definition
values with spaces (but NMake does).

.. include:: /include/COMPILE_DEFINITIONS_DISCLAIMER.txt
