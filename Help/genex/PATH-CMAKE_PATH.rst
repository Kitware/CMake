PATH:CMAKE_PATH
---------------

.. |genex-family| replace:: :ref:`genex-family-genex-path-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<PATH:CMAKE_PATH[,NORMALIZE],path...>

.. versionadded:: 3.24

Returns ``path``. If ``path`` is a native path, it is converted into a
cmake-style path with forward-slashes (``/``). On Windows, the long filename
marker is taken into account.

When the ``NORMALIZE`` option is specified, the path is :ref:`normalized
<Normalization>` after the conversion.
