PATH:HAS_EXTENSION
------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-queries`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:HAS_EXTENSION,path>

Returns ``1`` if ``path`` has an extension, ``0`` otherwise. An extension
starts at the left-most period in the filename and includes that period. A
leading period is ignored, and ``.`` and ``..`` have no extension.
