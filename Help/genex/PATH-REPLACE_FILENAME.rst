PATH:REPLACE_FILENAME
---------------------

.. |genex-family| replace:: :ref:`genex-family-genex-path-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<PATH:REPLACE_FILENAME,path...,input>

.. versionadded:: 3.24

Returns ``path`` with the filename component replaced by ``input``. If ``path``
has no filename component (i.e. ``$<PATH:HAS_FILENAME>`` returns ``0``),
``path`` is unchanged.

See :command:`cmake_path(REPLACE_FILENAME)` for more details.
