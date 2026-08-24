CMAKE_AUTOGEN_ORIGIN_DEPENDS
----------------------------

.. versionadded:: 3.14

Switch for forwarding origin target dependencies to the corresponding
:ref:`<ORIGIN>_autogen <<ORIGIN>_autogen>` targets.

  .. note::

    If the
    :ref:`<ORIGIN>_autogen_timestamp_deps <<ORIGIN>_autogen_timestamp_deps>`
    target is created, additional target dependencies are added to it instead
    of the :ref:`<ORIGIN>_autogen <<ORIGIN>_autogen>` target.  When using the
    :ref:`Visual Studio Generators`, the ``moc`` and ``uic`` step may be part
    of the ``<ORIGIN>`` target itself, in which case the dependencies are added
    to ``<ORIGIN>``.  See the :manual:`cmake-qt(7)` manual for details.

This variable is used to initialize the :prop_tgt:`AUTOGEN_ORIGIN_DEPENDS`
property on all the targets.  See that target property for additional
information.

By default ``CMAKE_AUTOGEN_ORIGIN_DEPENDS`` is ``ON``.
