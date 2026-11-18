LIST:FIND
---------

.. versionadded:: 3.27

.. |genex-family| replace:: :ref:`genex-family-genex-list-queries`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<LIST:FIND,list,value>

The index of the first item in ``list`` with the specified ``value``, or -1 if
``value`` is not in the ``list``.
