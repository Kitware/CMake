PATH:APPEND
-----------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:APPEND,path...,input,...>

Returns all the ``input`` arguments appended to ``path`` using ``/`` as the
``directory-separator``. Depending on the ``input``, the value of ``path`` may
be discarded.

See :command:`cmake_path(APPEND)` for more details.

.. versionchanged:: 3.27
  CMake now accepts a list of paths as argument. When a list of paths is
   specified, the operation will be applied to each path.
