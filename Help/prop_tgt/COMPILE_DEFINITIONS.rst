COMPILE_DEFINITIONS
-------------------

Preprocessor definitions for compiling a target's sources.

The COMPILE_DEFINITIONS property may be set to a semicolon-separated
list of preprocessor definitions using the syntax VAR or VAR=value.
Function-style definitions are not supported.  CMake will
automatically escape the value correctly for the native build system
(note that CMake language syntax may require escapes to specify some
values).  This property may be set on a per-configuration basis using
the name COMPILE_DEFINITIONS_<CONFIG> where <CONFIG> is an upper-case
name (ex.  "COMPILE_DEFINITIONS_DEBUG").

CMake will automatically drop some definitions that are not supported
by the native build tool.  The VS6 IDE does not support definition
values with spaces (but NMake does).

Contents of COMPILE_DEFINITIONS may use "generator expressions" with the
syntax "$<...>".  See the :manual:`cmake-generator-expressions(7)` manual
for available expressions.

.. include:: /include/COMPILE_DEFINITIONS_DISCLAIMER.txt
