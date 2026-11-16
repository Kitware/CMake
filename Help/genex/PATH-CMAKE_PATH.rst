PATH:CMAKE_PATH
---------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:CMAKE_PATH[,NORMALIZE],path...>

Returns ``path``. If ``path`` is a native path, it is converted into a
cmake-style path with forward-slashes (``/``). On Windows, the long filename
marker is taken into account.

When the ``NORMALIZE`` option is specified, the path is :ref:`normalized
<Normalization>` after the conversion.

.. versionchanged:: 3.27
  CMake now accepts a list of paths as argument. When a list of paths is
   specified, the operation will be applied to each path.
