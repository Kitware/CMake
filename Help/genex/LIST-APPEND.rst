LIST:APPEND
-----------

.. |genex-family| replace:: :ref:`genex-family-genex-list-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<LIST:APPEND,list,item,...>

.. versionadded:: 3.27

The ``list`` with each ``item`` appended.  Multiple items should be separated by
commas.
