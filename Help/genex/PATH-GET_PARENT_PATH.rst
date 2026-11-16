PATH:GET_PARENT_PATH
--------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-path-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:GET_PARENT_PATH,path...>

Returns the parent path of ``path``. If the path has a relative part, the
result has one fewer item. Otherwise, ``path`` is returned unchanged, so a
root directory is its own parent.

.. versionchanged:: 3.27
  CMake now accepts a list of paths as argument. When a list of paths is
   specified, the operation will be applied to each path.

If a requested component is not present in the path, an empty string is
returned.
