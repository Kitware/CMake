PATH:GET_ROOT_PATH
------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-path-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:GET_ROOT_PATH,path...>

Returns the root path of ``path``, formed by its root name followed by its
root directory separator. Either component may be empty. If neither is
present, an empty string is returned.

.. versionchanged:: 3.27
  CMake now accepts a list of paths as argument. When a list of paths is
   specified, the operation will be applied to each path.

If a requested component is not present in the path, an empty string is
returned.
