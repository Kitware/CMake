LIST:INSERT
-----------

.. |genex-family| replace:: :ref:`genex-family-genex-list-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<LIST:INSERT,list,index,item,...>

.. versionadded:: 3.27

The ``list`` with the ``item`` (or multiple items) inserted at the specified
``index``.  Multiple items should be separated by commas.

It is an error to specify an out-of-range ``index``. Valid indexes are 0 to N,
where N is the length of the list, inclusive. An empty list has length 0.
