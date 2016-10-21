MAP_IMPORTED_CONFIG_<CONFIG>
----------------------------

Map from project configuration to
:ref:`imported target <IMPORTED targets>`'s configuration.

Set this to the list of configurations of an imported target that may
be used for the current project's ``<CONFIG>`` configuration.  Targets
imported from another project may not provide the same set of
configuration names available in the current project.  Setting this
property tells CMake what imported configurations are suitable for use
when building the ``<CONFIG>`` configuration.  The first configuration in
the list found to be provided by the imported target (i.e. via
:prop_tgt:`IMPORTED_LOCATION_<CONFIG>` for the mapped-to ``<CONFIG>``)
is selected.  As a special case, an empty list element refers to the
configuration-less imported target location
(i.e. :prop_tgt:`IMPORTED_LOCATION`).

If this property is set and no matching configurations are available,
then the imported target is considered to be not found.  This property
is ignored for non-imported targets.

This property is initialized by the value of the
:variable:`CMAKE_MAP_IMPORTED_CONFIG_<CONFIG>` variable if it is set when a
target is created.
