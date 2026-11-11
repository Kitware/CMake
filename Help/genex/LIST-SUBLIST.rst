LIST:SUBLIST
------------

Syntax
^^^^^^

.. code-block:: cmake

  $<LIST:SUBLIST,list,begin,length>

.. versionadded:: 3.27

A sublist of the given ``list``.  If ``length`` is 0, an empty list will be
returned.  If ``length`` is -1 or the list is smaller than ``begin + length``,
the remaining items of the list starting at ``begin`` will be returned.
