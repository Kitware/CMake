LIST:REMOVE_ITEM
----------------

Syntax
^^^^^^

.. code-block:: cmake

  $<LIST:REMOVE_ITEM,list,value,...>

.. versionadded:: 3.27

The ``list`` with all instances of the given ``value`` (or values) removed. If
multiple values are given, they should be separated by commas.
