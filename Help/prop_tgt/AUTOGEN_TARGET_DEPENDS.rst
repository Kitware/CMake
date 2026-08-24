AUTOGEN_TARGET_DEPENDS
----------------------

Additional target dependencies of the corresponding
:ref:`<ORIGIN>_autogen <<ORIGIN>_autogen>` target.

  .. note::

    If the
    :ref:`<ORIGIN>_autogen_timestamp_deps <<ORIGIN>_autogen_timestamp_deps>`
    target is created, additional target dependencies are added to it instead
    of the :ref:`<ORIGIN>_autogen <<ORIGIN>_autogen>` target.  When using the
    :ref:`Visual Studio Generators`, the ``moc`` and ``uic`` step may be part
    of the ``<ORIGIN>`` target itself, in which case the dependencies are added
    to ``<ORIGIN>``.  See the :manual:`cmake-qt(7)` manual for details.


Targets which have their :prop_tgt:`AUTOMOC` or :prop_tgt:`AUTOUIC` property
``ON`` have a corresponding :ref:`<ORIGIN>_autogen <<ORIGIN>_autogen>`  target
which generates ``moc`` and ``uic`` files.
As this :ref:`<ORIGIN>_autogen <<ORIGIN>_autogen>` target is created
at generate-time, it is not possible to define dependencies of it using e.g.
:command:`add_dependencies`.  Instead the ``AUTOGEN_TARGET_DEPENDS`` target
property can be set to a :ref:`semicolon-separated list <CMake Language Lists>`
of additional dependencies for the :ref:`<ORIGIN>_autogen <<ORIGIN>_autogen>`
target. Dependencies can be target names or file names.

In total, the dependencies of the :ref:`<ORIGIN>_autogen <<ORIGIN>_autogen>`
target are composed from

- forwarded origin target dependencies
  (enabled by default via :prop_tgt:`AUTOGEN_ORIGIN_DEPENDS`)
- additional user defined dependencies from ``AUTOGEN_TARGET_DEPENDS``

See the :manual:`cmake-qt(7)` manual for more information on using CMake
with Qt.

Use cases
^^^^^^^^^

If :prop_tgt:`AUTOMOC` or :prop_tgt:`AUTOUIC` depends on a file that is either

- a :prop_sf:`GENERATED` non C++ file (e.g. a :prop_sf:`GENERATED` ``.json``
  or ``.ui`` file) or
- a :prop_sf:`GENERATED` C++ file that isn't recognized by :prop_tgt:`AUTOMOC`
  and :prop_tgt:`AUTOUIC` because it's skipped by :prop_sf:`SKIP_AUTOMOC`,
  :prop_sf:`SKIP_AUTOUIC`, :prop_sf:`SKIP_AUTOGEN` or :policy:`CMP0071` or
- a file that isn't in the origin target's sources

it must be added to ``AUTOGEN_TARGET_DEPENDS``.
