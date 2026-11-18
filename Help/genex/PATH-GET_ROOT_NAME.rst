PATH:GET_ROOT_NAME
------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-path-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:GET_ROOT_NAME,path...>

Returns the root name of ``path``. A root name identifies the filesystem root
on filesystems with multiple roots, such as ``C:`` or ``//myserver``. If the
path has no root name, an empty string is returned.

.. versionchanged:: 3.27
  CMake now accepts a list of paths as argument. When a list of paths is
   specified, the operation will be applied to each path.

If a requested component is not present in the path, an empty string is
returned.
