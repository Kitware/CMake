PATH_EQUAL
----------

.. |genex-family| replace:: :ref:`genex-family-path-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<PATH_EQUAL:path1,path2>

.. versionadded:: 3.24

Compares the lexical representations of two paths. No normalization is performed
on either path. Returns ``1`` if the paths are equal, ``0`` otherwise.

See :ref:`cmake_path(COMPARE) <Path Comparison>` for more details.
