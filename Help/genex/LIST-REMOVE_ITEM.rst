LIST:REMOVE_ITEM
----------------

.. versionadded:: 3.27

.. |genex-family| replace:: :ref:`genex-family-genex-list-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<LIST:REMOVE_ITEM,list,value,...>

The ``list`` with all instances of the given ``value`` (or values) removed. If
multiple values are given, they should be separated by commas.
