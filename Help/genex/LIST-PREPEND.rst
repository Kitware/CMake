LIST:PREPEND
------------

Syntax
^^^^^^

.. code-block:: cmake

  $<LIST:PREPEND,list,item,...>

.. versionadded:: 3.27

The ``list`` with each ``item`` inserted at the beginning.  If there are
multiple items, they should be separated by commas, and the order of the
prepended items will be preserved.
