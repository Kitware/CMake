PATH:APPEND
-----------

.. |genex-family| replace:: :ref:`genex-family-genex-path-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<PATH:APPEND,path...,input,...>

.. versionadded:: 3.24

Returns all the ``input`` arguments appended to ``path`` using ``/`` as the
``directory-separator``. Depending on the ``input``, the value of ``path`` may
be discarded.

See :command:`cmake_path(APPEND)` for more details.
