target_include_directories
--------------------------

Add include directories to a target.

::

  target_include_directories(<target> [SYSTEM] [BEFORE] <INTERFACE|PUBLIC|PRIVATE> [items1...]
    [<INTERFACE|PUBLIC|PRIVATE> [items2...] ...])

Specify include directories or targets to use when compiling a given
target.  The named <target> must have been created by a command such
as add_executable or add_library and must not be an IMPORTED target.

If BEFORE is specified, the content will be prepended to the property
instead of being appended.

The INTERFACE, PUBLIC and PRIVATE keywords are required to specify the
scope of the following arguments.  PRIVATE and PUBLIC items will
populate the INCLUDE_DIRECTORIES property of <target>.  PUBLIC and
INTERFACE items will populate the INTERFACE_INCLUDE_DIRECTORIES
property of <target>.  The following arguments specify include
directories.  Specified include directories may be absolute paths or
relative paths.  Repeated calls for the same <target> append items in
the order called.If SYSTEM is specified, the compiler will be told the
directories are meant as system include directories on some platforms
(signalling this setting might achieve effects such as the compiler
skipping warnings, or these fixed-install system files not being
considered in dependency calculations - see compiler docs).  If SYSTEM
is used together with PUBLIC or INTERFACE, the
INTERFACE_SYSTEM_INCLUDE_DIRECTORIES target property will be populated
with the specified directories.

Arguments to target_include_directories may use "generator
expressions" with the syntax "$<...>".
See the :manual:`cmake-generator-expressions(7)` manual for available
expressions.
