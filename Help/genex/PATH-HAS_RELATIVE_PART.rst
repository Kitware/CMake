PATH:HAS_RELATIVE_PART
----------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-queries`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:HAS_RELATIVE_PART,path>

Returns ``1`` if ``path`` has a relative part, ``0`` otherwise. The relative
part is the path with its root path removed.
