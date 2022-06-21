IMPORTED_NO_SYSTEM
------------------

.. versionadded:: 3.23

.. deprecated:: 3.25

  ``IMPORTED_NO_SYSTEM`` is deprecated. Set :prop_tgt:`SYSTEM` to `OFF`
  instead if you don't want target's include directories to be ``SYSTEM``
  when compiling consumers. Set :prop_tgt:`EXPORT_NO_SYSTEM` to `ON` instead
  if you don't want the include directories of the imported target generated
  by :command:`install(EXPORT)` and :command:`export` commands to be
  ``SYSTEM`` when compiling consumers.

Specifies that an :ref:`Imported Target <Imported Targets>` is not
a ``SYSTEM`` library.  This has the following effects:

* Entries of :prop_tgt:`INTERFACE_INCLUDE_DIRECTORIES` are not treated
  as ``SYSTEM`` include directories when compiling consumers (regardless of
  the value of the consumed target's :prop_tgt:`SYSTEM` property), as they
  would be by default.   Entries of
  :prop_tgt:`INTERFACE_SYSTEM_INCLUDE_DIRECTORIES` are not affected,
  and will always be treated as ``SYSTEM`` include directories.

This property can also be enabled on a non-imported target.  Doing so does
not affect the build system, but does tell the :command:`install(EXPORT)` and
:command:`export` commands to enable it on the imported targets they generate.

See the :prop_tgt:`NO_SYSTEM_FROM_IMPORTED` target property to set this
behavior on the target consuming the include directories rather than
providing them.
