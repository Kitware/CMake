PATH:REMOVE_EXTENSION
---------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:REMOVE_EXTENSION[,LAST_ONLY],path...>

Returns ``path`` with the :ref:`extension <EXTENSION_DEF>` removed, if any.

See :command:`cmake_path(REMOVE_EXTENSION)` for more details.

.. versionchanged:: 3.27
  CMake now accepts a list of paths as argument. When a list of paths is
   specified, the operation will be applied to each path.
