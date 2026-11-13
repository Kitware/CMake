PATH:REMOVE_EXTENSION
---------------------

.. |genex-family| replace:: :ref:`genex-family-genex-path-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<PATH:REMOVE_EXTENSION[,LAST_ONLY],path...>

.. versionadded:: 3.24

Returns ``path`` with the :ref:`extension <EXTENSION_DEF>` removed, if any.

See :command:`cmake_path(REMOVE_EXTENSION)` for more details.
