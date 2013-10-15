COMPILE_DEFINITIONS
-------------------

Preprocessor definitions for compiling a source file.

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
values with spaces (but NMake does).  Xcode does not support
per-configuration definitions on source files.

Disclaimer: Most native build tools have poor support for escaping
certain values.  CMake has work-arounds for many cases but some values
may just not be possible to pass correctly.  If a value does not seem
to be escaped correctly, do not attempt to work-around the problem by
adding escape sequences to the value.  Your work-around may break in a
future version of CMake that has improved escape support.  Instead
consider defining the macro in a (configured) header file.  Then
report the limitation.  Known limitations include:

::

  #          - broken almost everywhere
  ;          - broken in VS IDE 7.0 and Borland Makefiles
  ,          - broken in VS IDE
  %          - broken in some cases in NMake
  & |        - broken in some cases on MinGW
  ^ < > \"   - broken in most Make tools on Windows

CMake does not reject these values outright because they do work in
some cases.  Use with caution.
