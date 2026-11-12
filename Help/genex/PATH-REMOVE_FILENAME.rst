PATH:REMOVE_FILENAME
--------------------

.. |genex-family| replace:: :ref:`genex-family-genex-path-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<PATH:REMOVE_FILENAME,path...>

.. versionadded:: 3.24

Returns ``path`` with filename component (as returned by
``$<PATH:GET_FILENAME>``) removed. After removal, any trailing
``directory-separator`` is left alone, if present.

See :command:`cmake_path(REMOVE_FILENAME)` for more details.
