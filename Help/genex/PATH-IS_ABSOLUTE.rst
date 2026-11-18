PATH:IS_ABSOLUTE
----------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-queries`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:IS_ABSOLUTE,path>

Returns ``1`` if the path is absolute according to
:command:`cmake_path(IS_ABSOLUTE)`, ``0`` otherwise.
