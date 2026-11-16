PATH:RELATIVE_PATH
------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:RELATIVE_PATH,path...,base_directory>

Returns ``path``, modified to make it relative to the ``base_directory``
argument.

See :command:`cmake_path(RELATIVE_PATH)` for more details.

.. versionchanged:: 3.27
  CMake now accepts a list of paths as argument. When a list of paths is
   specified, the operation will be applied to each path.
