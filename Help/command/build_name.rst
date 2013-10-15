build_name
----------

Deprecated.  Use ${CMAKE_SYSTEM} and ${CMAKE_CXX_COMPILER} instead.

::

  build_name(variable)

Sets the specified variable to a string representing the platform and
compiler settings.  These values are now available through the
CMAKE_SYSTEM and CMAKE_CXX_COMPILER variables.
