CMAKE_DEBUG_TARGET_PROPERTIES
-----------------------------

Enables tracing output for target properties.

This variable can be populated with a list of properties to generate
debug output for when evaluating target properties.  Currently it can
only be used when evaluating the INCLUDE_DIRECTORIES,
COMPILE_DEFINITIONS and COMPILE_OPTIONS target properties.  In that
case, it outputs a backtrace for each entry in the target property.
Default is unset.
