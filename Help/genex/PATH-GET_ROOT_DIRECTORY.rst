PATH:GET_ROOT_DIRECTORY
-----------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-path-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:GET_ROOT_DIRECTORY,path...>

Returns the root directory separator of ``path``. This separator occurs at
the beginning of an absolute path, after any root name. If ``path`` has no
root directory separator, an empty string is returned.

.. versionchanged:: 3.27
  CMake now accepts a list of paths as argument. When a list of paths is
   specified, the operation will be applied to each path.

If a requested component is not present in the path, an empty string is
returned.
