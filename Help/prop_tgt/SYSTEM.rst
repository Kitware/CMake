SYSTEM
------

.. versionadded:: 3.25

Specifies that a target is a ``SYSTEM`` library.  This has the following effects:

* Entries of :prop_tgt:`INTERFACE_INCLUDE_DIRECTORIES` are treated as ``SYSTEM``
  include directories when compiling consumers.
  Entries of :prop_tgt:`INTERFACE_SYSTEM_INCLUDE_DIRECTORIES` are not affected,
  and will always be treated as ``SYSTEM`` include directories.

For imported targets, this property has a default value `ON`, which means that their
`INTERFACE_INCLUDE_DIRECTORIES` are treated as ``SYSTEM`` by default. If their
`SYSTEM` property is `OFF`, then their `INTERFACE_INCLUDE_DIRECTORIES` will not be
treated as ``SYSTEM``, regardless of the value of `IMPORTED_NO_SYSTEM` property.
