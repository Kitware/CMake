PATH:ABSOLUTE_PATH
------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:ABSOLUTE_PATH[,NORMALIZE],path...,base_directory>

Returns ``path`` as absolute. If ``path`` is a relative path
(:genex:`$<PATH:IS_RELATIVE> <PATH:IS_RELATIVE>` returns ``1``), it is
evaluated relative to the given base directory specified by ``base_directory``
argument.

When the ``NORMALIZE`` option is specified, the path is :ref:`normalized
<Normalization>` after the path computation.

See :command:`cmake_path(ABSOLUTE_PATH)` for more details.

.. versionchanged:: 3.27
  CMake now accepts a list of paths as argument. When a list of paths is
   specified, the operation will be applied to each path.
