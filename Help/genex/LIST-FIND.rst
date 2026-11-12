LIST:FIND
---------

.. |genex-family| replace:: :ref:`genex-family-genex-list-queries`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<LIST:FIND,list,value>

.. versionadded:: 3.27

The index of the first item in ``list`` with the specified ``value``, or -1 if
``value`` is not in the ``list``.
