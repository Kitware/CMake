PATH_EQUAL
----------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-path-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH_EQUAL:path1,path2>

Compares the lexical representations of two paths. No normalization is performed
on either path. Returns ``1`` if the paths are equal, ``0`` otherwise.

See :ref:`cmake_path(COMPARE) <Path Comparison>` for more details.
