EXPORT_NO_SYSTEM
----------------

.. versionadded:: 3.25

Specifies that :command:`install(EXPORT)` and :command:`export` commands will
generate an imported target with :prop_tgt:`SYSTEM` property `OFF`.

See the :prop_tgt:`NO_SYSTEM_FROM_IMPORTED` target property to set this
behavior on the target *consuming* the include directories rather than the
one *providing* them.
