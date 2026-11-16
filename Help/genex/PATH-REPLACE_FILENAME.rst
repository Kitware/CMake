PATH:REPLACE_FILENAME
---------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:REPLACE_FILENAME,path...,input>

Returns ``path`` with the filename component replaced by ``input``. If ``path``
has no filename component (i.e. :genex:`$<PATH:HAS_FILENAME>
<PATH:HAS_FILENAME>` returns ``0``),
``path`` is unchanged.

See :command:`cmake_path(REPLACE_FILENAME)` for more details.

.. versionchanged:: 3.27
  CMake now accepts a list of paths as argument. When a list of paths is
   specified, the operation will be applied to each path.
