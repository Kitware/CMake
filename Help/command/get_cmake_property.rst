get_cmake_property
------------------

Get a property of the CMake instance.

::

  get_cmake_property(VAR property)

Get a property from the CMake instance.  The value of the property is
stored in the variable ``VAR``.  If the property is not found, ``VAR``
will be  set to "NOTFOUND".  See the :manual:`cmake-properties(7)` manual
for available properties.

See also the more general :command:`get_property` command.
