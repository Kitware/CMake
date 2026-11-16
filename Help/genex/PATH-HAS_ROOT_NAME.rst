PATH:HAS_ROOT_NAME
------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-queries`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:HAS_ROOT_NAME,path>

Returns ``1`` if ``path`` has a root name, ``0`` otherwise. A root name
identifies the filesystem root when a filesystem has multiple roots, such as
``C:`` or ``//myserver``.
