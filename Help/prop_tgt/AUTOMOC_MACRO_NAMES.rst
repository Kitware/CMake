AUTOMOC_MACRO_NAMES
-------------------

Additional macro names used by :prop_tgt:`AUTOMOC`
to determine if a C++ file needs to be processed by ``moc``.

This property is only used if the :prop_tgt:`AUTOMOC` property is ``ON``
for this target.

CMake searches for the strings ``Q_OBJECT`` and ``Q_GADGET`` to
determine if a file needs to be processed by ``moc``.
:prop_tgt:`AUTOMOC_MACRO_NAMES` allows to add additional strings to the
search list. This is useful for cases where the ``Q_OBJECT`` or ``Q_GADGET``
macro is hidden inside another macro.

By default :prop_tgt:`AUTOMOC_MACRO_NAMES` is initialized from
:variable:`CMAKE_AUTOMOC_MACRO_NAMES`, which is empty by default.

See the :manual:`cmake-qt(7)` manual for more information on using CMake
with Qt.

Example
-------
In this case the the ``Q_OBJECT`` macro is hidden inside an other macro
called ``CUSTOM_MACRO``. To let CMake know that source files, that contain
``CUSTOM_MACRO``, need to be ``moc`` processed, we call::

  set_property(TARGET tgt PROPERTY AUTOMOC_MACRO_NAMES "CUSTOM_MACRO")
