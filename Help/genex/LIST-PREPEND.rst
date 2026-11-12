LIST:PREPEND
------------

.. |genex-family| replace:: :ref:`genex-family-genex-list-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<LIST:PREPEND,list,item,...>

.. versionadded:: 3.27

The ``list`` with each ``item`` inserted at the beginning.  If there are
multiple items, they should be separated by commas, and the order of the
prepended items will be preserved.
