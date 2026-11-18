LIST:PREPEND
------------

.. versionadded:: 3.27

.. |genex-family| replace:: :ref:`genex-family-genex-list-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<LIST:PREPEND,list,item,...>

The ``list`` with each ``item`` inserted at the beginning.  If there are
multiple items, they should be separated by commas, and the order of the
prepended items will be preserved.
