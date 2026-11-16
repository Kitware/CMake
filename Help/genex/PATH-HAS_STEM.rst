PATH:HAS_STEM
-------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-queries`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:HAS_STEM,path>

Returns ``1`` if ``path`` has a stem, ``0`` otherwise.  The stem is the part
of the filename before its extension.
