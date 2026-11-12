CONFIG
------

.. |genex-family| replace:: :ref:`genex-family-configuration-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<CONFIG>

Configuration name. Use this instead of the deprecated :genex:`CONFIGURATION`
generator expression.

Syntax
^^^^^^

.. code-block:: cmake

  $<CONFIG:cfgs>

``1`` if config is any one of the entries in comma-separated list ``cfgs``, else
``0``. This is a case-insensitive comparison. The mapping in
:prop_tgt:`MAP_IMPORTED_CONFIG_<CONFIG>` is also considered by this expression
when it is evaluated on a property of an :prop_tgt:`IMPORTED` target.

.. versionchanged:: 3.19
  Multiple configurations can be specified for ``cfgs``.
  CMake 3.18 and earlier only accepted a single configuration.
