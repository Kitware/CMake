IMPORTED_NO_SYSTEM
------------------

.. versionadded:: 3.23

Specifies that an :ref:`Imported Target <Imported Targets>` is not
a ``SYSTEM`` library.  This has the following effects:

* Entries of :prop_tgt:`INTERFACE_INCLUDE_DIRECTORIES` are not treated
  as ``SYSTEM`` include directories when compiling consumers, as they
  would be by default.   Entries of
  :prop_tgt:`INTERFACE_SYSTEM_INCLUDE_DIRECTORIES` are not affected,
  and will always be treated as ``SYSTEM`` include directories.

This property can also be enabled on a non-imported target.  Doing so does
not affect the build system, but does tell the :command:`install(EXPORT)` and
:command:`export` commands to enable it on the imported targets they generate.

See the :prop_tgt:`NO_SYSTEM_FROM_IMPORTED` target property to set this
behavior on the target consuming the include directories rather than
providing them.
