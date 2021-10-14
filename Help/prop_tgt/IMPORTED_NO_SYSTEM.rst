IMPORTED_NO_SYSTEM
------------------

Specifies that an :ref:`Imported Target <Imported Targets>` is not
a ``SYSTEM`` library.  This has the following effects:

* Entries of :prop_tgt:`INTERFACE_INCLUDE_DIRECTORIES` are not treated
  as ``SYSTEM`` include directories when compiling consumers, as they
  would be by default.

Ignored for non-imported targets.

See the :prop_tgt:`NO_SYSTEM_FROM_IMPORTED` target property to set this
behavior on the target consuming the include directories rather than
providing them.
