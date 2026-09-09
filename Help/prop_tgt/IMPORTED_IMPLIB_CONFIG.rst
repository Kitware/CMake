IMPORTED_IMPLIB_<CONFIG>
------------------------

Full path to the ``<CONFIG>``-specific import library for an
:ref:`IMPORTED <Imported Targets>` target.  This is the
``<CONFIG>``-specific version of the :prop_tgt:`IMPORTED_IMPLIB`
property.

The ``<CONFIG>`` name corresponds to that provided by the project from
which the target is imported.  The :prop_tgt:`MAP_IMPORTED_CONFIG_<CONFIG>`
target property may be used to map between a project's configurations and
those of an imported target.

Each ``<CONFIG>`` for which this property is set should also be
listed in the :prop_tgt:`IMPORTED_CONFIGURATIONS` target property.
