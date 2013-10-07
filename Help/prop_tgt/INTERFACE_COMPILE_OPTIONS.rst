INTERFACE_COMPILE_OPTIONS
-------------------------

List of interface options to pass to the compiler.

Targets may populate this property to publish the compile options
required to compile against the headers for the target.  Consuming
targets can add entries to their own COMPILE_OPTIONS property such as
$<TARGET_PROPERTY:foo,INTERFACE_COMPILE_OPTIONS> to use the compile
options specified in the interface of 'foo'.

This property also supports generator expressions.  See the
:manual:`cmake-generator-expressions(7)` manual for available expressions.
