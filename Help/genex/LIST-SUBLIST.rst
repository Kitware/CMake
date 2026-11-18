LIST:SUBLIST
------------

.. versionadded:: 3.27

.. |genex-family| replace:: :ref:`genex-family-genex-list-queries`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<LIST:SUBLIST,list,begin,length>

A sublist of the given ``list``.  If ``length`` is 0, an empty list will be
returned.  If ``length`` is -1 or the list is smaller than ``begin + length``,
the remaining items of the list starting at ``begin`` will be returned.
