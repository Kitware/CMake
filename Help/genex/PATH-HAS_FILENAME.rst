PATH:HAS_FILENAME
-----------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-queries`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:HAS_FILENAME,path>

Returns ``1`` if ``path`` has a filename, ``0`` otherwise. A path has a
filename when it does not end in a directory separator; the filename is its
last item name.
