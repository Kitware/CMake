LIST:APPEND
-----------

.. versionadded:: 3.27

.. |genex-family| replace:: :ref:`genex-family-genex-list-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<LIST:APPEND,list,item,...>

The ``list`` with each ``item`` appended.  Multiple items should be separated by
commas.
