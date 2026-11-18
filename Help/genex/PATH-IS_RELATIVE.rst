PATH:IS_RELATIVE
----------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-queries`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:IS_RELATIVE,path>

Returns ``1`` if the path is relative according to
:command:`cmake_path(IS_RELATIVE)`, ``0`` otherwise.
