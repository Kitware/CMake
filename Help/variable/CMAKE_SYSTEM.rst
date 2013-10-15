CMAKE_SYSTEM
------------

Name of system cmake is compiling for.

This variable is the composite of CMAKE_SYSTEM_NAME and
CMAKE_SYSTEM_VERSION, like this
${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_VERSION}.  If CMAKE_SYSTEM_VERSION
is not set, then CMAKE_SYSTEM is the same as CMAKE_SYSTEM_NAME.
