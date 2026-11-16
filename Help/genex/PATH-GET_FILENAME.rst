PATH:GET_FILENAME
-----------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-path-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:GET_FILENAME,path...>

Returns the filename of ``path``. A path has a filename when it does not end
with a directory separator; the filename is its last item name. If ``path``
ends with a directory separator, an empty string is returned.

.. versionchanged:: 3.27
  CMake now accepts a list of paths as argument. When a list of paths is
   specified, the operation will be applied to each path.

If a requested component is not present in the path, an empty string is
returned.
