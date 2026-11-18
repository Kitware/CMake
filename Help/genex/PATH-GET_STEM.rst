PATH:GET_STEM
-------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-path-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:GET_STEM,path...>

Returns the stem of the filename in ``path``. The stem is the part of the
filename before its extension. With ``LAST_ONLY``, the extension is taken to
start at the right-most period instead of the left-most. If the path has no
filename, an empty string is returned.

.. versionchanged:: 3.27
  CMake now accepts a list of paths as argument. When a list of paths is
   specified, the operation will be applied to each path.

If a requested component is not present in the path, an empty string is
returned.
