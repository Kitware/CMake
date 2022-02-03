NO_SYSTEM_FROM_IMPORTED
-----------------------

Do not treat include directories from the interfaces of consumed
:ref:`imported targets` as ``SYSTEM``.

The contents of the :prop_tgt:`INTERFACE_INCLUDE_DIRECTORIES` target property
of imported targets are treated as ``SYSTEM`` includes by default.  If this
property is enabled on a target, compilation of sources in that target will
not treat the contents of the ``INTERFACE_INCLUDE_DIRECTORIES`` of consumed
imported targets as system includes.  Either way, entries of
:prop_tgt:`INTERFACE_SYSTEM_INCLUDE_DIRECTORIES` are not affected,
and will always be treated as ``SYSTEM`` include directories.

This property is initialized by the value of the
:variable:`CMAKE_NO_SYSTEM_FROM_IMPORTED` variable if it is set when a target
is created.

See the :prop_tgt:`IMPORTED_NO_SYSTEM` target property to set this behavior
on the target providing the include directories rather than consuming them.
