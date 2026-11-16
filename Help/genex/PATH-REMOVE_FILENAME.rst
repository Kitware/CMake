PATH:REMOVE_FILENAME
--------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:REMOVE_FILENAME,path...>

Returns ``path`` with filename component (as returned by
:genex:`$<PATH:GET_FILENAME> <PATH:GET_FILENAME>`) removed. After
removal, any trailing ``directory-separator`` is left alone, if present.

See :command:`cmake_path(REMOVE_FILENAME)` for more details.

.. versionchanged:: 3.27
  CMake now accepts a list of paths as argument. When a list of paths is
   specified, the operation will be applied to each path.
