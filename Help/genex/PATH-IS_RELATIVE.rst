PATH:IS_RELATIVE
----------------

.. |genex-family| replace:: :ref:`genex-family-genex-path-queries`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<PATH:IS_RELATIVE,path>

.. versionadded:: 3.24

Returns ``1`` if the path is relative according to
:command:`cmake_path(IS_RELATIVE)`, ``0`` otherwise.
