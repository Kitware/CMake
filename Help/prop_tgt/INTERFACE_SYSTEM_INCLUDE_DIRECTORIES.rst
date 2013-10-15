INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
------------------------------------

List of public system include directories for a library.

Targets may populate this property to publish the include directories
which contain system headers, and therefore should not result in
compiler warnings.  Consuming targets will then mark the same include
directories as system headers.

This property also supports generator expressions.  See the
:manual:`cmake-generator-expressions(7)` manual for available expressions.
