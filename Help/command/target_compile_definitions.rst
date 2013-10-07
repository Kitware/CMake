target_compile_definitions
--------------------------

Add compile definitions to a target.

::

  target_compile_definitions(<target> <INTERFACE|PUBLIC|PRIVATE> [items1...]
    [<INTERFACE|PUBLIC|PRIVATE> [items2...] ...])

Specify compile definitions to use when compiling a given target.  The
named <target> must have been created by a command such as
add_executable or add_library and must not be an IMPORTED target.  The
INTERFACE, PUBLIC and PRIVATE keywords are required to specify the
scope of the following arguments.  PRIVATE and PUBLIC items will
populate the COMPILE_DEFINITIONS property of <target>.  PUBLIC and
INTERFACE items will populate the INTERFACE_COMPILE_DEFINITIONS
property of <target>.  The following arguments specify compile
definitions.  Repeated calls for the same <target> append items in the
order called.

Arguments to target_compile_definitions may use "generator expressions" with
the syntax "$<...>".  See the :manual:`cmake-generator-expressions(7)` manual
for available expressions.
