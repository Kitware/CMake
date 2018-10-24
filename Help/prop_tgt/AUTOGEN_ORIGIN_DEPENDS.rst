AUTOGEN_ORIGIN_DEPENDS
----------------------

Switch for forwarding origin target dependencies to the corresponding
``_autogen`` target.

Targets which have their :prop_tgt:`AUTOMOC` or :prop_tgt:`AUTOUIC` property
``ON`` have a corresponding ``_autogen`` target which is used to auto generate
``moc`` and ``uic`` files.  As this ``_autogen`` target is created at
generate-time, it is not possible to define dependencies of it,
such as to create inputs for the ``moc`` or ``uic`` executable.

The dependencies of the ``_autogen`` target are composed from

- the origin target dependencies
  (by default enabled via :prop_tgt:`AUTOGEN_ORIGIN_DEPENDS`)
- user defined dependencies from :prop_tgt:`AUTOGEN_TARGET_DEPENDS`

:prop_tgt:`AUTOGEN_ORIGIN_DEPENDS` decides whether the origin target
dependencies should be forwarded to the ``_autogen`` target or not.

By default :prop_tgt:`AUTOGEN_ORIGIN_DEPENDS` is initialized from
:variable:`CMAKE_AUTOGEN_ORIGIN_DEPENDS` which is ``ON`` by default.

See the :manual:`cmake-qt(7)` manual for more information on using CMake
with Qt.
