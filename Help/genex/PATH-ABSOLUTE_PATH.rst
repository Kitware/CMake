PATH:ABSOLUTE_PATH
------------------

.. |genex-family| replace:: :ref:`genex-family-genex-path-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<PATH:ABSOLUTE_PATH[,NORMALIZE],path...,base_directory>

.. versionadded:: 3.24

Returns ``path`` as absolute. If ``path`` is a relative path
(``$<PATH:IS_RELATIVE>`` returns ``1``), it is evaluated relative to the given
base directory specified by ``base_directory`` argument.

When the ``NORMALIZE`` option is specified, the path is :ref:`normalized
<Normalization>` after the path computation.

See :command:`cmake_path(ABSOLUTE_PATH)` for more details.
