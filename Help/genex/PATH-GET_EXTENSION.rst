PATH:GET_EXTENSION
------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-path-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:GET_EXTENSION[,LAST_ONLY],path...>

Returns the extension of the filename in ``path``. By default, the extension
starts at the left-most period and includes that period. With ``LAST_ONLY``,
it starts at the right-most period. A leading period is ignored, and ``.``
and ``..`` have no extension. If there is no extension, an empty string is
returned.

.. versionchanged:: 3.27
  CMake now accepts a list of paths as argument. When a list of paths is
   specified, the operation will be applied to each path.

If a requested component is not present in the path, an empty string is
returned.
